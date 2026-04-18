#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "case_io.h"
#include "waveguide.h"
#include "matrix_solver.h"

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

    const std::string status = sim_case.study_kind == StudyKind::FIELD_MAP_PREPARATION
                                   ? "pending_field_reconstruction"
                                   : "prototype_dispersion_csv_generated";

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

void write_field_map_sampling_grid(const SimulationCase& sim_case,
                                   const Waveguide& wg,
                                   const std::filesystem::path& output_path) {
    const std::filesystem::path grid_path = output_path / sim_case.output.canonical_csv_name;
    std::ofstream grid_file(grid_path);
    if (!grid_file) {
        throw std::runtime_error("Não foi possível criar o CSV canônico do grid de amostragem.");
    }

    const std::filesystem::path legacy_path = output_path / "results.csv";

    grid_file << "sample_id,x,y,refractive_index,k_squared,requested_component,status\n";
    grid_file << std::setprecision(16);

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

    std::size_t sample_id = 0;
    for (std::size_t iy = 0; iy < sim_case.field_map.sample_ny; ++iy) {
        for (std::size_t ix = 0; ix < sim_case.field_map.sample_nx; ++ix) {
            const double x = x_min + ix * dx;
            const double y = y_min + iy * dy;
            grid_file << sample_id++ << ","
                      << x << ","
                      << y << ","
                      << wg.get_refractive_index(x, y) << ","
                      << wg.get_k_squared(x, y) << ","
                      << sim_case.field_map.component << ","
                      << "pending_field_reconstruction\n";
        }
    }

    grid_file.close();
    copy_file_to_if_different(grid_path, legacy_path);

    std::ofstream status_file(output_path / "field_map_status.txt");
    if (!status_file) {
        throw std::runtime_error("Não foi possível criar field_map_status.txt.");
    }

    status_file << "status: pending_field_reconstruction\n";
    status_file << "reason: a reconstrucao do campo ainda nao foi implementada no solver vetorial.\n";
    status_file << "requested_component: " << sim_case.field_map.component << "\n";
    status_file << "lambda0: " << sim_case.field_map.lambda0 << "\n";
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
                  << " por segmentos explícitos; a formulação vetorial completa do artigo ainda não"
                  << " está fechada."
                  << std::endl;
        std::cout << "Quadratura de fronteira: "
                  << boundary_quadrature_model_to_cstr(sim_case.assembly_options.boundary_quadrature_model)
                  << " com " << sim_case.assembly_options.boundary_subdivisions
                  << " subdivisões" << std::endl;

        WaveguideParams params = sim_case.waveguide;
        const Discretization discretization = sim_case.discretization;
        const std::filesystem::path output_path(output_dir);

        bool profile_written = false;

        if (sim_case.study_kind == StudyKind::FIELD_MAP_PREPARATION) {
            params.lambda0 = sim_case.field_map.lambda0;
            Waveguide wg(params, discretization);
            if (!profile_written) {
                write_profile_samples(wg, (output_path / "profile_samples.csv").string());
            }
            write_field_map_sampling_grid(sim_case, wg, output_path);
        } else {
            const std::filesystem::path canonical_csv_path = output_path / sim_case.output.canonical_csv_name;
            std::ofstream outfile(canonical_csv_path);
            if (!outfile) {
                throw std::runtime_error("Não foi possível criar o arquivo de resultados em " + output_dir);
            }

            outfile << "case_id,article_figure,target_mode,V_param,lambda0,beta,B_norm,det_abs\n";
            outfile << std::setprecision(16);

            const double tolerance = sim_case.sweep.v_step * 0.5;

            for (double V = sim_case.sweep.v_start; V <= sim_case.sweep.v_end + tolerance; V += sim_case.sweep.v_step) {
                const double numerator = params.a * 0.5 * std::sqrt(params.n2m * params.n2m - params.n3 * params.n3);
                params.lambda0 = 2.0 * PI * numerator / V;
                const double k0 = 2.0 * PI / params.lambda0;

                std::cout << "Calculando para V = " << V
                          << " (lambda0 = " << params.lambda0 * 1e6 << " um)" << std::endl;

                Waveguide wg(params, discretization);
                if (!profile_written) {
                    write_profile_samples(wg, (output_path / "profile_samples.csv").string());
                    profile_written = true;
                }

                const double beta_min = k0 * params.n3;
                const double beta_max = k0 * params.n2m;
                const double beta_found = find_beta_root(wg, beta_min, beta_max, sim_case.assembly_options);
                const double det_abs = calculate_determinant_magnitude(beta_found, wg, sim_case.assembly_options);

                const double beta_norm = beta_found / k0;
                const double B_norm = ((beta_norm * beta_norm) - params.n3 * params.n3) /
                                      (params.n2m * params.n2m - params.n3 * params.n3);

                std::cout << "  -> beta encontrado: " << beta_found
                          << ", B = " << B_norm
                          << ", |det(A)| = " << det_abs
                          << std::endl;

                outfile << sim_case.case_id << ","
                        << sim_case.article_figure << ","
                        << sim_case.target_mode << ","
                        << V << ","
                        << params.lambda0 << ","
                        << beta_found << ","
                        << B_norm << ","
                        << det_abs << "\n";
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
