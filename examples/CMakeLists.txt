
cmake_minimum_required(VERSION 3.28)

add_executable(example-liant-mod
    src/00_module_usage.cpp
)
target_link_libraries(example-liant-mod PUBLIC liant-mod)
