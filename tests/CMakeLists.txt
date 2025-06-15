
cmake_minimum_required(VERSION 3.28)

include(FetchContent)

FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG 1da23a3e8119ec5cce4f9388e91b065e20bf06f5 # v2.4.12
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(doctest)

#
# Regular 'runtime' tests
#
add_executable(test-liant
    src/data.hpp
    src/single_container.cpp
    src/base_container.cpp
    src/dependencies.cpp
)

target_link_libraries(test-liant liant doctest::doctest_with_main)
target_compile_definitions(test-liant PUBLIC
    DOCTEST_CONFIG_VOID_CAST_EXPRESSIONS
)

#
# Make tests discoverable by CTest
#
list(APPEND CMAKE_MODULE_PATH ${doctest_SOURCE_DIR}/scripts/cmake)
include(doctest)
doctest_discover_tests(test-liant)