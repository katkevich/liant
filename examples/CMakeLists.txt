
cmake_minimum_required(VERSION 3.28)

# Liant as a C++23 module and C++23 'import std'
add_executable(example_liant_mod
    src/00_liant_module_import_std.cpp
)
target_compile_features(example_liant_mod PRIVATE cxx_std_23)
set_target_properties(example_liant_mod PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
    CXX_MODULE_STD ON
)
target_link_libraries(example_liant_mod PUBLIC liant::liant_module)


# Liant as a C++20 module and standard headers in global module fragment
add_executable(example_liant_mod_nostd
    src/01_liant_module_no_import_std.cpp
)
target_compile_features(example_liant_mod_nostd PRIVATE cxx_std_20)
set_target_properties(example_liant_mod_nostd PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
target_link_libraries(example_liant_mod_nostd PUBLIC liant::liant_module_nostd)
