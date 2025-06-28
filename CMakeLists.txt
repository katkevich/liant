cmake_minimum_required(VERSION 3.30)

# This specific value changes as experimental support evolves.
# See `Help/dev/experimental.rst` in the CMake source (https://cmake.org/download/) corresponding to
# your CMake build for the exact value to use.
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

project(liant VERSION 1.0.0)

#
# common settings for liant/liant_mod/liant_mod_nostd targets
#
add_library(liant_common INTERFACE)
target_compile_options(liant_common INTERFACE
    $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU,Intel>: -Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:
        # Various options for closer standard conformance
        /utf-8 /Zc:__cplusplus /Zc:throwingNew /Zc:inline /Zc:externConstexpr
        /Zc:templateScope /Zc:checkGwOdr /Zc:enumTypes
        /W4
        /wd4459 # local variable name hides global variable
        /wd4702 # unreachable code
        /wd4100 # unreferenced formal parameter
    >
)

#
# conventional, header-based target
#
add_library(liant INTERFACE)
add_library(liant::liant ALIAS liant)
target_sources(liant INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/liant/liant.hpp
        include/liant/container.hpp
        include/liant/dependency.hpp
        include/liant/dependency_macro.hpp
        include/liant/ptr.hpp
        include/liant/tuple.hpp
        include/liant/typelist.hpp
        include/liant/snake_case.hpp
)
target_compile_features(liant INTERFACE cxx_std_20)
set_target_properties(liant PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    CXX_SCAN_FOR_MODULES OFF
)
target_link_libraries(liant INTERFACE liant_common)

#
# C++23 module with 'import std'
# Only tested with libc++-20 (libstdc++ don't support 'import std' for now)
#
# Configure this CMake target with '-DCMAKE_CXX_FLAGS="-stdlib=libc++"'
#
add_library(liant_mod STATIC)
add_library(liant::liant_mod ALIAS liant_mod)
target_compile_features(liant_mod PUBLIC cxx_std_23)
set_target_properties(liant_mod PROPERTIES
    CXX_STANDARD_REQUIRED ON
    # Currently CMake requires extensions enabled when using import std.
    # https://gitlab.kitware.com/cmake/cmake/-/issues/25916
    # https://gitlab.kitware.com/cmake/cmake/-/issues/25539
    CXX_EXTENSIONS ON
    CXX_MODULE_STD ON
)
target_sources(liant_mod PRIVATE
    FILE_SET privateHeaders TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/liant/liant.hpp
        include/liant/export_macro.hpp
        include/liant/container.hpp
        include/liant/dependency.hpp
        include/liant/ptr.hpp
        include/liant/tuple.hpp
        include/liant/typelist.hpp
        include/liant/snake_case.hpp
)
target_sources(liant_mod PUBLIC
    FILE_SET publicHeaders TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/liant/dependency_macro.hpp
)
target_sources(liant_mod PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
    FILES
        src/liant_std.ixx
)
target_link_libraries(liant_mod PRIVATE liant_common)
target_compile_definitions(liant_mod PRIVATE LIANT_MODULE LIANT_STD_MODULE)

#
# C++20 module with STD in global-module-fragment
#
add_library(liant_mod_nostd STATIC)
add_library(liant::liant_mod_nostd ALIAS liant_mod_nostd)
target_compile_features(liant_mod_nostd PUBLIC cxx_std_23)
set_target_properties(liant_mod_nostd PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
target_sources(liant_mod_nostd PRIVATE
    FILE_SET privateHeaders TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/liant/liant.hpp
        include/liant/export_macro.hpp
        include/liant/container.hpp
        include/liant/dependency.hpp
        include/liant/ptr.hpp
        include/liant/tuple.hpp
        include/liant/typelist.hpp
        include/liant/snake_case.hpp
)
target_sources(liant_mod_nostd PUBLIC
    FILE_SET publicHeaders TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/liant/dependency_macro.hpp
)
target_sources(liant_mod_nostd PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
    FILES
        src/liant_nostd.ixx
)
target_link_libraries(liant_mod_nostd PRIVATE liant_common)
target_compile_definitions(liant_mod_nostd PRIVATE LIANT_MODULE)

#
# Test ON/OFF, Examples ON/OFF
#
option(LIANT_BUILD_TESTS "Build Liant tests" ${PROJECT_IS_TOP_LEVEL})
option(LIANT_BUILD_EXAMPLES "Build Liant examples" ${PROJECT_IS_TOP_LEVEL})

#
# Tests
#
if (LIANT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests EXCLUDE_FROM_ALL)
endif()

#
# Examples
#
if (LIANT_BUILD_EXAMPLES)
    add_subdirectory(examples EXCLUDE_FROM_ALL)
endif()


#
# Library installation (only header-base library installed is implemented)
#
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(LIANT_INSTALL_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/liant)

# generate version file
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/liant-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
    ARCH_INDEPENDENT
)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/liant-config.cmake.in"
    "${PROJECT_BINARY_DIR}/liant-config.cmake"
    INSTALL_DESTINATION ${LIANT_INSTALL_CMAKE_DIR}
)

# set target installation location properties and associates it with the targets files
install(
    TARGETS liant_common
    EXPORT liant-targets
)
install(
    TARGETS liant
    EXPORT liant-targets
    FILE_SET HEADERS
)

# install the targets files
install(
    EXPORT liant-targets
    NAMESPACE liant::
    DESTINATION ${LIANT_INSTALL_CMAKE_DIR}
)

# install the config and version files
install(
    FILES
        "${PROJECT_BINARY_DIR}/liant-config.cmake"
        "${PROJECT_BINARY_DIR}/liant-config-version.cmake"
    DESTINATION ${LIANT_INSTALL_CMAKE_DIR}
)
