#pragma once

#include <string>

#include "matrix_solver.h"
#include "waveguide.h"

enum class StudyKind {
    DISPERSION_CURVE,
    FIELD_MAP
};

struct SweepRange {
    double v_start;
    double v_end;
    double v_step;
};

struct OutputSpec {
    std::string family = "cases";
    std::string figure_id = "prototype";
    std::string curve_id = "eq_integral";
    std::string canonical_csv_name = "results.csv";
};

struct FieldMapSpec {
    double lambda0 = 0.0;
    double beta_over_k0 = 0.0;
    std::size_t sample_nx = 0;
    std::size_t sample_ny = 0;
    std::string component = "Ey";
};

struct SimulationCase {
    std::string case_id;
    std::string article_section;
    std::string article_figure;
    std::string target_mode;
    std::string notes;
    StudyKind study_kind = StudyKind::DISPERSION_CURVE;
    WaveguideParams waveguide;
    Discretization discretization;
    SweepRange sweep;
    AssemblyOptions assembly_options;
    OutputSpec output;
    FieldMapSpec field_map;
};

SimulationCase load_case_from_json(const std::string& path);
std::string profile_type_to_string(ProfileType profile_type);
std::string study_kind_to_string(StudyKind study_kind);
void prepare_case_output(const SimulationCase& sim_case,
                         const std::string& input_path,
                         const std::string& output_dir);
