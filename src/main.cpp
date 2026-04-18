#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "case_io.h"
#include "waveguide.h"
#include "matrix_solver.h"

namespace {

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

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <arquivo_de_caso.json> [diretorio_saida]" << std::endl;
        return 1;
    }

    try {
        const std::string case_file = argv[1];
        const SimulationCase sim_case = load_case_from_json(case_file);
        const std::string output_dir = argc >= 3 ? argv[2] : ("out/" + sim_case.case_id);

        prepare_case_output(sim_case, case_file, output_dir);

        std::cout << "Iniciando simulação do caso '" << sim_case.case_id << "'..." << std::endl;
        std::cout << "Referência alvo: " << sim_case.article_figure
                  << " (" << sim_case.article_section << ")" << std::endl;
        std::cout << "Aviso: o núcleo integral e a busca modal ainda estão em estágio de protótipo."
                  << std::endl;

        WaveguideParams params = sim_case.waveguide;
        const Discretization discretization = sim_case.discretization;
        const std::filesystem::path output_path(output_dir);

        std::ofstream outfile(output_path / "results.csv");
        if (!outfile) {
            throw std::runtime_error("Não foi possível criar o arquivo de resultados em " + output_dir);
        }

        outfile << "case_id,article_figure,target_mode,V_param,lambda0,beta,B_norm\n";
        outfile << std::setprecision(16);

        bool profile_written = false;
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
            const double beta_found = find_beta_root(wg, beta_min, beta_max);

            const double beta_norm = beta_found / k0;
            const double B_norm = ((beta_norm * beta_norm) - params.n3 * params.n3) /
                                  (params.n2m * params.n2m - params.n3 * params.n3);

            std::cout << "  -> beta encontrado: " << beta_found << ", B = " << B_norm << std::endl;

            outfile << sim_case.case_id << ","
                    << sim_case.article_figure << ","
                    << sim_case.target_mode << ","
                    << V << ","
                    << params.lambda0 << ","
                    << beta_found << ","
                    << B_norm << "\n";
        }

        std::cout << "Simulação concluída. Resultados em " << (output_path / "results.csv") << std::endl;
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Erro durante a execução: " << error.what() << std::endl;
        return 1;
    }
}
