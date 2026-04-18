#include "case_io.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {

std::string read_text_file(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Não foi possível abrir o arquivo de caso: " + path);
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::size_t find_json_key_position(const std::string& text, const std::string& key) {
    const std::string token = "\"" + key + "\"";
    std::size_t search_position = 0;

    while (true) {
        const std::size_t position = text.find(token, search_position);
        if (position == std::string::npos) {
            return std::string::npos;
        }

        const std::size_t after_token = position + token.size();
        const std::size_t next_non_space = text.find_first_not_of(" \t\r\n", after_token);
        if (next_non_space != std::string::npos && text[next_non_space] == ':') {
            return position;
        }

        search_position = after_token;
    }
}

std::size_t find_key_position(const std::string& text, const std::string& key) {
    const std::size_t position = find_json_key_position(text, key);
    if (position == std::string::npos) {
        throw std::runtime_error("Chave obrigatória ausente no JSON: " + key);
    }
    return position;
}

std::size_t find_optional_key_position(const std::string& text, const std::string& key) {
    return find_json_key_position(text, key);
}

bool has_key(const std::string& text, const std::string& key) {
    return find_optional_key_position(text, key) != std::string::npos;
}

std::string extract_object(const std::string& text, const std::string& key) {
    const std::size_t key_position = find_key_position(text, key);
    const std::size_t brace_start = text.find('{', key_position);
    if (brace_start == std::string::npos) {
        throw std::runtime_error("Objeto JSON inválido para a chave: " + key);
    }

    int depth = 0;
    for (std::size_t i = brace_start; i < text.size(); ++i) {
        if (text[i] == '{') {
            ++depth;
        } else if (text[i] == '}') {
            --depth;
            if (depth == 0) {
                return text.substr(brace_start, i - brace_start + 1);
            }
        }
    }

    throw std::runtime_error("Objeto JSON não fechado para a chave: " + key);
}

std::string extract_optional_object(const std::string& text, const std::string& key) {
    if (!has_key(text, key)) {
        return {};
    }
    return extract_object(text, key);
}

std::string extract_raw_value(const std::string& text, const std::string& key) {
    const std::size_t key_position = find_key_position(text, key);
    const std::size_t colon_position = text.find(':', key_position);
    if (colon_position == std::string::npos) {
        throw std::runtime_error("Valor ausente para a chave: " + key);
    }

    std::size_t value_start = text.find_first_not_of(" \t\r\n", colon_position + 1);
    if (value_start == std::string::npos) {
        throw std::runtime_error("Valor vazio para a chave: " + key);
    }

    if (text[value_start] == '"') {
        const std::size_t value_end = text.find('"', value_start + 1);
        if (value_end == std::string::npos) {
            throw std::runtime_error("String não terminada para a chave: " + key);
        }
        return text.substr(value_start + 1, value_end - value_start - 1);
    }

    std::size_t value_end = text.find_first_of(",}\r\n", value_start);
    if (value_end == std::string::npos) {
        value_end = text.size();
    }

    return text.substr(value_start, value_end - value_start);
}

double extract_double(const std::string& text, const std::string& key) {
    return std::stod(extract_raw_value(text, key));
}

std::size_t extract_size(const std::string& text, const std::string& key) {
    return static_cast<std::size_t>(std::stoul(extract_raw_value(text, key)));
}

std::string extract_string(const std::string& text, const std::string& key) {
    return extract_raw_value(text, key);
}

bool extract_bool(const std::string& text, const std::string& key) {
    const std::string value = extract_raw_value(text, key);
    if (value == "true") {
        return true;
    }
    if (value == "false") {
        return false;
    }

    throw std::runtime_error("Valor booleano inválido para a chave: " + key);
}

ProfileType parse_profile_type(const std::string& value) {
    if (value == "homogeneous") {
        return ProfileType::HOMOGENEOUS;
    }
    if (value == "parabolic_1d") {
        return ProfileType::PARABOLIC_1D;
    }
    if (value == "circular_2d") {
        return ProfileType::CIRCULAR_2D;
    }

    throw std::runtime_error("Tipo de perfil desconhecido: " + value);
}

BoundaryQuadratureModel parse_boundary_quadrature_model(const std::string& value) {
    if (value == "midpoint") {
        return BoundaryQuadratureModel::MIDPOINT;
    }
    if (value == "gauss2") {
        return BoundaryQuadratureModel::GAUSS2;
    }

    throw std::runtime_error("Modelo de quadratura de fronteira desconhecido: " + value);
}

StudyKind parse_study_kind(const std::string& value) {
    if (value == "dispersion_curve") {
        return StudyKind::DISPERSION_CURVE;
    }
    if (value == "field_map" || value == "field_map_preparation") {
        return StudyKind::FIELD_MAP;
    }

    throw std::runtime_error("Tipo de estudo desconhecido: " + value);
}

void validate_case(const SimulationCase& sim_case) {
    if (sim_case.case_id.empty()) {
        throw std::runtime_error("case_id não pode ser vazio.");
    }
    if (sim_case.output.canonical_csv_name.empty()) {
        throw std::runtime_error("output.canonical_csv_name não pode ser vazio.");
    }
    if (sim_case.output.family == "figures") {
        if (sim_case.output.figure_id.empty()) {
            throw std::runtime_error("Casos da família figures exigem output.figure_id.");
        }
        if (sim_case.output.curve_id.empty()) {
            throw std::runtime_error("Casos da família figures exigem output.curve_id.");
        }
    }
    if (sim_case.waveguide.a <= 0.0 || sim_case.waveguide.b <= 0.0) {
        throw std::runtime_error("As dimensões geométricas a e b devem ser positivas.");
    }
    if (sim_case.waveguide.n2m <= sim_case.waveguide.n3) {
        throw std::runtime_error("Espera-se n2m > n3 para o intervalo modal guiado protótipo.");
    }
    if (sim_case.discretization.Nx == 0 || sim_case.discretization.Ny == 0) {
        throw std::runtime_error("A discretização deve ter Nx e Ny positivos.");
    }
    if (sim_case.assembly_options.boundary_subdivisions == 0) {
        throw std::runtime_error("boundary_subdivisions deve ser positivo.");
    }
    if (sim_case.study_kind == StudyKind::DISPERSION_CURVE) {
        if (sim_case.sweep.v_start <= 0.0 || sim_case.sweep.v_end < sim_case.sweep.v_start ||
            sim_case.sweep.v_step <= 0.0) {
            throw std::runtime_error(
                "Casos de dispersão exigem sweep com v_start > 0, v_end >= v_start e v_step > 0.");
        }
    }
    if (sim_case.study_kind == StudyKind::FIELD_MAP) {
        if (sim_case.field_map.lambda0 <= 0.0) {
            throw std::runtime_error("field_map.lambda0 deve ser positivo.");
        }
        if (sim_case.field_map.beta_over_k0 < 0.0) {
            throw std::runtime_error("field_map.beta_over_k0 não pode ser negativo.");
        }
        if (sim_case.field_map.sample_nx == 0 || sim_case.field_map.sample_ny == 0) {
            throw std::runtime_error("field_map.sample_nx e field_map.sample_ny devem ser positivos.");
        }
    }
}

void write_summary_file(const SimulationCase& sim_case,
                        const std::string& input_path,
                        const std::filesystem::path& output_dir) {
    std::ofstream summary(output_dir / "run_summary.txt");
    if (!summary) {
        throw std::runtime_error("Não foi possível criar run_summary.txt em " + output_dir.string());
    }

    summary << "case_id: " << sim_case.case_id << "\n";
    summary << "input_file: " << input_path << "\n";
    summary << "article_section: " << sim_case.article_section << "\n";
    summary << "article_figure: " << sim_case.article_figure << "\n";
    summary << "target_mode: " << sim_case.target_mode << "\n";
    summary << "profile_type: " << profile_type_to_string(sim_case.waveguide.profile_type) << "\n";
    summary << "study_kind: " << study_kind_to_string(sim_case.study_kind) << "\n";
    summary << "output_family: " << sim_case.output.family << "\n";
    summary << "figure_id: " << sim_case.output.figure_id << "\n";
    summary << "curve_id: " << sim_case.output.curve_id << "\n";
    summary << "canonical_csv_name: " << sim_case.output.canonical_csv_name << "\n";
    summary << "Nx: " << sim_case.discretization.Nx << "\n";
    summary << "Ny: " << sim_case.discretization.Ny << "\n";
    if (sim_case.study_kind == StudyKind::DISPERSION_CURVE) {
        summary << "article_x_range: [" << sim_case.sweep.v_start << ", " << sim_case.sweep.v_end
                << "] step " << sim_case.sweep.v_step << "\n";
    } else {
        summary << "article_x_range: not_applicable\n";
    }
    summary << "include_scalar_contrast: " << (sim_case.assembly_options.include_scalar_contrast ? "true" : "false")
            << "\n";
    summary << "include_regular_gradient: " << (sim_case.assembly_options.include_regular_gradient ? "true" : "false")
            << "\n";
    summary << "include_boundary_distribution: "
            << (sim_case.assembly_options.include_boundary_distribution ? "true" : "false") << "\n";
    summary << "boundary_quadrature_model: "
            << boundary_quadrature_model_to_cstr(sim_case.assembly_options.boundary_quadrature_model) << "\n";
    summary << "boundary_subdivisions: " << sim_case.assembly_options.boundary_subdivisions << "\n";
    if (sim_case.study_kind == StudyKind::FIELD_MAP) {
        summary << "field_map_lambda0: " << sim_case.field_map.lambda0 << "\n";
        summary << "field_map_beta_over_k0: " << sim_case.field_map.beta_over_k0 << "\n";
        summary << "field_map_sample_nx: " << sim_case.field_map.sample_nx << "\n";
        summary << "field_map_sample_ny: " << sim_case.field_map.sample_ny << "\n";
        summary << "field_map_component: " << sim_case.field_map.component << "\n";
    }
    summary << "notes: " << sim_case.notes << "\n";
    summary << "solver_status: boundary_segments_operator_with_null_vector_mode_estimation\n";
    summary << "limitations: G^S e G^NS escalares ja estao implementadas no regime guiado com y >= 0 e y' >= 0; a montagem atual usa o termo (k^2-k3^2)G, a parte volumetrica regular de eps*grad(1/eps) multiplicando grad'G e uma aproximacao explicita do termo de fronteira por segmentos da borda da malha; o vetor modal e estimado por menor singularidade do operador discretizado, mas a formulacao vetorial completa do artigo, um tratamento mais rigoroso da singularidade/quadratura da fronteira e a busca rigorosa dos zeros exatos de det(A) ainda permanecem pendentes.\n";
}

} // namespace

SimulationCase load_case_from_json(const std::string& path) {
    const std::string text = read_text_file(path);
    const std::string reference = extract_object(text, "reference");
    const std::string materials = extract_object(text, "materials");
    const std::string geometry = extract_object(text, "geometry");
    const std::string discretization = extract_object(text, "discretization");
    const std::string solver = extract_optional_object(text, "solver");
    const std::string study = extract_optional_object(text, "study");
    const std::string output = extract_optional_object(text, "output");
    const std::string field_map = extract_optional_object(text, "field_map");

    SimulationCase sim_case;
    sim_case.case_id = extract_string(text, "case_id");
    sim_case.article_section = extract_string(reference, "article_section");
    sim_case.article_figure = extract_string(reference, "article_figure");
    sim_case.target_mode = extract_string(reference, "target_mode");
    sim_case.notes = extract_string(text, "notes");

    if (!study.empty() && has_key(study, "kind")) {
        sim_case.study_kind = parse_study_kind(extract_string(study, "kind"));
    }

    sim_case.waveguide.n1 = extract_double(materials, "n1");
    sim_case.waveguide.n3 = extract_double(materials, "n3");
    sim_case.waveguide.n2m = extract_double(materials, "n2m");
    sim_case.waveguide.a = extract_double(geometry, "a");
    sim_case.waveguide.b = extract_double(geometry, "b");
    sim_case.waveguide.lambda0 = 0.0;
    sim_case.waveguide.profile_type = parse_profile_type(extract_string(geometry, "profile_type"));

    sim_case.discretization.Nx = extract_size(discretization, "Nx");
    sim_case.discretization.Ny = extract_size(discretization, "Ny");

    if (sim_case.study_kind == StudyKind::DISPERSION_CURVE) {
        const std::string sweep = extract_object(text, "sweep");
        sim_case.sweep.v_start = extract_double(sweep, "v_start");
        sim_case.sweep.v_end = extract_double(sweep, "v_end");
        sim_case.sweep.v_step = extract_double(sweep, "v_step");
    }

    if (!solver.empty()) {
        if (has_key(solver, "include_scalar_contrast")) {
            sim_case.assembly_options.include_scalar_contrast = extract_bool(solver, "include_scalar_contrast");
        }
        if (has_key(solver, "include_regular_gradient")) {
            sim_case.assembly_options.include_regular_gradient = extract_bool(solver, "include_regular_gradient");
        }
        if (has_key(solver, "include_boundary_distribution")) {
            sim_case.assembly_options.include_boundary_distribution = extract_bool(solver, "include_boundary_distribution");
        }
        if (has_key(solver, "boundary_quadrature_model")) {
            sim_case.assembly_options.boundary_quadrature_model =
                parse_boundary_quadrature_model(extract_string(solver, "boundary_quadrature_model"));
        }
        if (has_key(solver, "boundary_subdivisions")) {
            sim_case.assembly_options.boundary_subdivisions = extract_size(solver, "boundary_subdivisions");
        }
    }

    if (!output.empty()) {
        if (has_key(output, "family")) {
            sim_case.output.family = extract_string(output, "family");
        }
        if (has_key(output, "figure_id")) {
            sim_case.output.figure_id = extract_string(output, "figure_id");
        }
        if (has_key(output, "curve_id")) {
            sim_case.output.curve_id = extract_string(output, "curve_id");
        }
        if (has_key(output, "canonical_csv_name")) {
            sim_case.output.canonical_csv_name = extract_string(output, "canonical_csv_name");
        }
    }

    if (!field_map.empty()) {
        if (has_key(field_map, "lambda0")) {
            sim_case.field_map.lambda0 = extract_double(field_map, "lambda0");
        }
        if (has_key(field_map, "beta_over_k0")) {
            sim_case.field_map.beta_over_k0 = extract_double(field_map, "beta_over_k0");
        }
        if (has_key(field_map, "sample_nx")) {
            sim_case.field_map.sample_nx = extract_size(field_map, "sample_nx");
        }
        if (has_key(field_map, "sample_ny")) {
            sim_case.field_map.sample_ny = extract_size(field_map, "sample_ny");
        }
        if (has_key(field_map, "component")) {
            sim_case.field_map.component = extract_string(field_map, "component");
        }
    }

    validate_case(sim_case);
    return sim_case;
}

std::string profile_type_to_string(ProfileType profile_type) {
    switch (profile_type) {
        case ProfileType::HOMOGENEOUS:
            return "homogeneous";
        case ProfileType::PARABOLIC_1D:
            return "parabolic_1d";
        case ProfileType::CIRCULAR_2D:
            return "circular_2d";
    }

    throw std::runtime_error("ProfileType inválido.");
}

std::string study_kind_to_string(StudyKind study_kind) {
    switch (study_kind) {
        case StudyKind::DISPERSION_CURVE:
            return "dispersion_curve";
        case StudyKind::FIELD_MAP:
            return "field_map";
    }

    throw std::runtime_error("StudyKind inválido.");
}

void prepare_case_output(const SimulationCase& sim_case,
                         const std::string& input_path,
                         const std::string& output_dir) {
    (void)sim_case;

    const std::filesystem::path output_path(output_dir);
    std::filesystem::create_directories(output_path);
    std::filesystem::copy_file(input_path,
                               output_path / "input_snapshot.json",
                               std::filesystem::copy_options::overwrite_existing);

    write_summary_file(sim_case, input_path, output_path);
}
