pichot-1982-diffused-channel-waveguide/
├── README.md
├── LICENSE
├── CITATION.cff
├── .gitignore
├── CMakeLists.txt
├── docs/
│   ├── 01_article_overview.md
│   ├── 02_physical_problem.md
│   ├── 03_from_helmholtz_to_integral_equation.md
│   ├── 04_moment_method_discretization.md
│   ├── 05_numerical_strategy.md
│   ├── 06_validation_plan.md
│   └── 07_reproduction_log.md
├── data/
│   ├── digitized/
│   └── reference/
├── cases/
│   ├── homogeneous_rectangular/
│   ├── homogeneous_channel/
│   ├── diffused_1d_parabolic/
│   └── diffused_2d_circular/
├── include/
├── src/
│   ├── main.cpp
│   ├── geometry.cpp
│   ├── profiles.cpp
│   ├── green_function.cpp
│   ├── integral_operator.cpp
│   ├── moment_matrix.cpp
│   ├── eigen_search.cpp
│   └── field_reconstruction.cpp
├── scripts/
│   ├── plot_dispersion.py
│   ├── compare_figures.py
│   └── digitize_curves.py
├── results/
│   ├── figures/
│   ├── csv/
│   └── logs/
└── tests/
    ├── test_profiles.cpp
    ├── test_green_function.cpp
    └── test_matrix_assembly.cpp
