#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "case_io.h"
#include "matrix_solver.h"
#include "waveguide.h"

namespace {

std::filesystem::path make_default_output_path(const SimulationCase& sim_case) {
    if (sim_case.output.family == "figures") {
        return std::filesystem::path("out") / "figures" / sim_case.output.figure_id / sim_case.case_id;
    }
    return std::filesystem::path("out") / sim_case.case_id;
}

void write_profile_samples(const Waveguide& wg, const std::string& output_path) {
    std::ofstream profile_file(output_path);
    if (!profile_file) {
        throw std::runtime_error("Não foi possível criar o arquivo de amostras do perfil: " + output_path);
    }

    profile_file << "cell_id,x_center,y_center,dx,dy,refractive_index,k_squared\n";
    profile_file << std::setprecision(16);

    for (const auto& cell : wg.get_cells()) {
        profile_file << cell.id << ","
                     << cell.cx << ","
                     << cell.cy << ","
                     << cell.dx << ","
                     << cell.dy << ","
                     << wg.get_refractive_index(cell.cx, cell.cy) << ","
                     << wg.get_k_squared(cell.cx, cell.cy) << "\n";
    }
}

void write_output_manifest(const SimulationCase& sim_case, const std::filesystem::path& output_path) {
    std::ofstream manifest(output_path / "output_manifest.json");
    if (!manifest) {
        throw std::runtime_error("Não foi possível criar output_manifest.json.");
    }

    const std::string status = sim_case.study_kind == StudyKind::FIELD_MAP
                                   ? "field_reconstructed_from_mode_solution"
                                   : "dispersion_csv_generated_with_modal_residual";

    manifest << "{\n";
    manifest << "  \"case_id\": \"" << sim_case.case_id << "\",\n";
    manifest << "  \"study_kind\": \"" << study_kind_to_string(sim_case.study_kind) << "\",\n";
    manifest << "  \"output_family\": \"" << sim_case.output.family << "\",\n";
    manifest << "  \"figure_id\": \"" << sim_case.output.figure_id << "\",\n";
    manifest << "  \"curve_id\": \"" << sim_case.output.curve_id << "\",\n";
    manifest << "  \"canonical_csv_name\": \"" << sim_case.output.canonical_csv_name << "\",\n";
    manifest << "  \"output_dir\": \"" << output_path.string() << "\",\n";
    manifest << "  \"canonical_csv_path\": \"" << (output_path / sim_case.output.canonical_csv_name).string() << "\",\n";
    manifest << "  \"results_csv_path\": \"" << (output_path / "results.csv").string() << "\",\n";
    manifest << "  \"profile_samples_path\": \"" << (output_path / "profile_samples.csv").string() << "\",\n";
    manifest << "  \"status\": \"" << status << "\"\n";
    manifest << "}\n";
}

void copy_file_to_if_different(const std::filesystem::path& source, const std::filesystem::path& target) {
    if (source == target) {
        return;
    }
    std::filesystem::copy_file(source, target, std::filesystem::copy_options::overwrite_existing);
}

double lambda0_from_article_x_parameter(double article_x_parameter, const WaveguideParams& params) {
    const double contrast = params.n2m * params.n2m - params.n3 * params.n3;
    if (article_x_parameter <= 0.0 || contrast <= 0.0) {
        throw std::runtime_error("Parâmetros inválidos ao converter a abcissa do artigo em lambda0.");
    }
    return 2.0 * params.b * std::sqrt(contrast) / article_x_parameter;
}

std::string normalize_component_label(const std::string& component) {
    if (component == "Ex" || component == "EX" || component == "ex") {
        return "Ex";
    }
    if (component == "Ey" || component == "EY" || component == "ey") {
        return "Ey";
    }
    throw std::runtime_error("Componente de campo não suportada: " + component);
}

const Cell* find_cell_containing_point(const Waveguide& wg, double x, double y) {
    for (const Cell& cell : wg.get_cells()) {
        const double x_left = cell.cx - 0.5 * cell.dx;
        const double x_right = cell.cx + 0.5 * cell.dx;
        const double y_bottom = cell.cy - 0.5 * cell.dy;
        const double y_top = cell.cy + 0.5 * cell.dy;
        if (x >= x_left && x <= x_right && y >= y_bottom && y <= y_top) {
            return &cell;
        }
    }
    return nullptr;
}

std::size_t component_offset(const std::string& component, std::size_t cell_count) {
    if (component == "Ex") {
        return 0;
    }
    if (component == "Ey") {
        return cell_count;
    }
    throw std::runtime_error("Componente de campo não suportada ao escolher o offset.");
}

std::vector<Complex> normalize_mode_by_component(const std::vector<Complex>& coefficients,
                                                 std::size_t cell_count,
                                                 const std::string& component) {
    const std::size_t offset = component_offset(component, cell_count);
    double max_component = 0.0;
    for (std::size_t cell_id = 0; cell_id < cell_count; ++cell_id) {
        max_component = std::max(max_component, std::abs(coefficients[offset + cell_id]));
    }

    if (max_component <= 0.0) {
        return coefficients;
    }

    std::vector<Complex> normalized = coefficients;
    for (Complex& value : normalized) {
        value /= max_component;
    }
    return normalized;
}

Complex sample_mode_component(const Waveguide& wg,
                              const std::vector<Complex>& coefficients,
                              const std::string& component,
                              double x,
                              double y) {
    const Cell* cell = find_cell_containing_point(wg, x, y);
    if (cell == nullptr) {
        return Complex(0.0, 0.0);
    }

    const std::size_t offset = component_offset(component, wg.get_cells().size());
    return coefficients.at(offset + cell->id);
}

void write_mode_coefficients(const Waveguide& wg,
                             const std::vector<Complex>& coefficients,
                             const std::filesystem::path& output_path) {
    std::ofstream mode_file(output_path / "mode_coefficients.csv");
    if (!mode_file) {
        throw std::runtime_error("Não foi possível criar mode_coefficients.csv.");
    }

    mode_file << "cell_id,x_center,y_center,Ex_real,Ex_imag,Ex_abs,Ey_real,Ey_imag,Ey_abs\n";
    mode_file << std::setprecision(16);

    const std::size_t cell_count = wg.get_cells().size();
    for (const Cell& cell : wg.get_cells()) {
        const Complex ex = coefficients.at(cell.id);
        const Complex ey = coefficients.at(cell_count + cell.id);
        mode_file << cell.id << ","
                  << cell.cx << ","
                  << cell.cy << ","
                  << ex.real() << ","
                  << ex.imag() << ","
                  << std::abs(ex) << ","
                  << ey.real() << ","
                  << ey.imag() << ","
                  << std::abs(ey) << "\n";
    }
}

void write_field_map_grid(const SimulationCase& sim_case,
                          const Waveguide& wg,
                          const std::vector<Complex>& coefficients,
                          double beta,
                          double modal_residual,
                          const std::filesystem::path& output_path) {
    const std::string component = normalize_component_label(sim_case.field_map.component);
    const std::vector<Complex> normalized_coefficients =
        normalize_mode_by_component(coefficients, wg.get_cells().size(), component);

    const std::filesystem::path field_map_path = output_path / sim_case.output.canonical_csv_name;
    std::ofstream field_map_file(field_map_path);
    if (!field_map_file) {
        throw std::runtime_error("Não foi possível criar o CSV canônico do mapa de campo.");
    }

    std::ofstream sampling_grid_file(output_path / "field_sampling_grid.csv");
    if (!sampling_grid_file) {
        throw std::runtime_error("Não foi possível criar field_sampling_grid.csv.");
    }

    const std::filesystem::path legacy_path = output_path / "results.csv";
    field_map_file << "sample_id,x,y,refractive_index,k_squared,requested_component,field_real,field_imag,field_abs,field_abs_normalized,beta,beta_over_k0,modal_residual\n";
    sampling_grid_file << "sample_id,x,y,refractive_index,k_squared,requested_component\n";
    field_map_file << std::setprecision(16);
    sampling_grid_file << std::setprecision(16);

    const double x_min = -0.5 * sim_case.waveguide.a;
    const double x_max = 0.5 * sim_case.waveguide.a;
    const double y_min = 0.0;
    const double y_max = sim_case.waveguide.b;
    const double dx = sim_case.field_map.sample_nx > 1
                          ? (x_max - x_min) / static_cast<double>(sim_case.field_map.sample_nx - 1)
                          : 0.0;
    const double dy = sim_case.field_map.sample_ny > 1
                          ? (y_max - y_min) / static_cast<double>(sim_case.field_map.sample_ny - 1)
                          : 0.0;
    const double k0 = wg.get_k0();

    std::size_t sample_id = 0;
    for (std::size_t iy = 0; iy < sim_case.field_map.sample_ny; ++iy) {
        for (std::size_t ix = 0; ix < sim_case.field_map.sample_nx; ++ix) {
            const double x = x_min + ix * dx;
            const double y = y_min + iy * dy;
            const Complex field_value = sample_mode_component(wg, normalized_coefficients, component, x, y);
            const double field_abs = std::abs(field_value);
            field_map_file << sample_id << ","
                           << x << ","
                           << y << ","
                           << wg.get_refractive_index(x, y) << ","
                           << wg.get_k_squared(x, y) << ","
                           << component << ","
                           << field_value.real() << ","
                           << field_value.imag() << ","
                           << field_abs << ","
                           << field_abs << ","
                           << beta << ","
                           << beta / k0 << ","
                           << modal_residual << "\n";
            sampling_grid_file << sample_id << ","
                               << x << ","
                               << y << ","
                               << wg.get_refractive_index(x, y) << ","
                               << wg.get_k_squared(x, y) << ","
                               << component << "\n";
            ++sample_id;
        }
    }

    field_map_file.close();
    sampling_grid_file.close();
    copy_file_to_if_different(field_map_path, legacy_path);

    std::ofstream status_file(output_path / "field_map_status.txt");
    if (!status_file) {
        throw std::runtime_error("Não foi possível criar field_map_status.txt.");
    }

    status_file << "status: reconstructed_from_mode_solution\n";
    status_file << "requested_component: " << component << "\n";
    status_file << "lambda0: " << sim_case.field_map.lambda0 << "\n";
    status_file << "beta: " << beta << "\n";
    status_file << "beta_over_k0: " << beta / wg.get_k0() << "\n";
    status_file << "modal_residual: " << modal_residual << "\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <arquivo_de_caso.json> [diretorio_saida]" << std::endl;
        return 1;
    }

    try {
        const std::string case_file = argv[1];
        const SimulationCase sim_case = load_case_from_json(case_file);
        const std::filesystem::path default_output_path = make_default_output_path(sim_case);
        const std::string output_dir = argc >= 3 ? argv[2] : default_output_path.string();

        prepare_case_output(sim_case, case_file, output_dir);
        write_output_manifest(sim_case, output_dir);

        std::cout << "Iniciando simulação do caso '" << sim_case.case_id << "'..." << std::endl;
        std::cout << "Referência alvo: " << sim_case.article_figure
                  << " (" << sim_case.article_section << ")" << std::endl;
        std::cout << "Aviso: a busca modal usa um operador protótipo com o termo escalar"
                  << " (k^2-k3^2)G, a parte volumétrica regular com grad'G e um termo de fronteira"
                  << " por segmentos explícitos; o campo modal agora é estimado por menor singularidade,"
                  << " mas a formulação vetorial completa do artigo ainda não está fechada."
                  << std::endl;
        std::cout << "Quadratura de fronteira: "
                  << boundary_quadrature_model_to_cstr(sim_case.assembly_options.boundary_quadrature_model)
                  << " com " << sim_case.assembly_options.boundary_subdivisions
                  << " subdivisões" << std::endl;

        WaveguideParams params = sim_case.waveguide;
        const Discretization discretization = sim_case.discretization;
        const std::filesystem::path output_path(output_dir);

        bool profile_written = false;

        if (sim_case.study_kind == StudyKind::FIELD_MAP) {
            params.lambda0 = sim_case.field_map.lambda0;
            Waveguide wg(params, discretization);
            if (!profile_written) {
                write_profile_samples(wg, (output_path / "profile_samples.csv").string());
                profile_written = true;
            }
            const double k0 = wg.get_k0();
            const double beta_min = k0 * params.n3;
            const double beta_max = k0 * params.n2m;
            double beta = sim_case.field_map.beta_over_k0 > 0.0
                              ? sim_case.field_map.beta_over_k0 * k0
                              : find_beta_root(wg, beta_min, beta_max, sim_case.assembly_options);
            if (sim_case.field_map.beta_over_k0 <= 0.0) {
                beta = refine_beta_with_modal_residual(beta, beta_min, beta_max, wg, sim_case.assembly_options);
            }
            const ModeSolution mode_solution = solve_mode_at_beta(beta, wg, sim_case.assembly_options);
            write_mode_coefficients(wg, mode_solution.coefficients, output_path);
            write_field_map_grid(
                sim_case, wg, mode_solution.coefficients, beta, mode_solution.modal_residual, output_path);
            std::cout << "  -> beta usado: " << beta
                      << ", beta/k0 = " << beta / k0
                      << ", residual modal = " << mode_solution.modal_residual
                      << std::endl;
        } else {
            const std::filesystem::path canonical_csv_path = output_path / sim_case.output.canonical_csv_name;
            std::ofstream outfile(canonical_csv_path);
            if (!outfile) {
                throw std::runtime_error("Não foi possível criar o arquivo de resultados em " + output_dir);
            }

            outfile << "case_id,article_figure,target_mode,article_x_param,lambda0,beta,beta_over_k0,normalized_beta,det_abs,modal_residual\n";
            outfile << std::setprecision(16);

            const double tolerance = sim_case.sweep.v_step * 0.5;

            for (double article_x_param = sim_case.sweep.v_start;
                 article_x_param <= sim_case.sweep.v_end + tolerance;
                 article_x_param += sim_case.sweep.v_step) {
                params.lambda0 = lambda0_from_article_x_parameter(article_x_param, params);
                const double k0 = 2.0 * PI / params.lambda0;

                std::cout << "Calculando para x_art = " << article_x_param
                          << " (lambda0 = " << params.lambda0 * 1e6 << " um)" << std::endl;

                Waveguide wg(params, discretization);
                if (!profile_written) {
                    write_profile_samples(wg, (output_path / "profile_samples.csv").string());
                    profile_written = true;
                }

                const double beta_min = k0 * params.n3;
                const double beta_max = k0 * params.n2m;
                double beta_found = find_beta_root(wg, beta_min, beta_max, sim_case.assembly_options);
                beta_found =
                    refine_beta_with_modal_residual(beta_found, beta_min, beta_max, wg, sim_case.assembly_options);
                const ModeSolution mode_solution = solve_mode_at_beta(beta_found, wg, sim_case.assembly_options);

                const double beta_over_k0 = beta_found / k0;
                const double normalized_beta = ((beta_over_k0 * beta_over_k0) - params.n3 * params.n3) /
                                               (params.n2m * params.n2m - params.n3 * params.n3);

                std::cout << "  -> beta encontrado: " << beta_found
                          << ", beta/k0 = " << beta_over_k0
                          << ", y_norm = " << normalized_beta
                          << ", |det(A)| = " << mode_solution.determinant_magnitude
                          << ", residual modal = " << mode_solution.modal_residual
                          << std::endl;

                outfile << sim_case.case_id << ","
                        << sim_case.article_figure << ","
                        << sim_case.target_mode << ","
                        << article_x_param << ","
                        << params.lambda0 << ","
                        << beta_found << ","
                        << beta_over_k0 << ","
                        << normalized_beta << ","
                        << mode_solution.determinant_magnitude << ","
                        << mode_solution.modal_residual << "\n";
            }

            outfile.close();
            copy_file_to_if_different(canonical_csv_path, output_path / "results.csv");
        }

        std::cout << "Simulação concluída. Resultados em " << (output_path / "results.csv") << std::endl;
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Erro durante a execução: " << error.what() << std::endl;
        return 1;
    }
}
