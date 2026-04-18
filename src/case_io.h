#pragma once

#include <string>

#include "matrix_solver.h"
#include "waveguide.h"

struct SweepRange {
    double v_start;
    double v_end;
    double v_step;
};

struct SimulationCase {
    std::string case_id;
    std::string article_section;
    std::string article_figure;
    std::string target_mode;
    std::string notes;
    WaveguideParams waveguide;
    Discretization discretization;
    SweepRange sweep;
    AssemblyOptions assembly_options;
};

SimulationCase load_case_from_json(const std::string& path);
std::string profile_type_to_string(ProfileType profile_type);
void prepare_case_output(const SimulationCase& sim_case,
                         const std::string& input_path,
                         const std::string& output_dir);
