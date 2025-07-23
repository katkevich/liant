cmake_minimum_required(VERSION 3.30)

# This specific value changes as experimental support evolves.
# See `Help/dev/experimental.rst` in the CMake source (https://cmake.org/download/) corresponding to
# your CMake build for the exact value to use.
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

project(liant VERSION 1.0.0)

#
# Common compiler options for liant/liant_module/liant_module_nostd targets
#
add_library(liant_warn_lvl INTERFACE)
add_library(liant::liant_warn_lvl ALIAS liant_warn_lvl)
target_compile_options(liant_warn_lvl INTERFACE
    $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU,Intel>: -Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:
        # Various options for closer standard conformance
        /Zc:__cplusplus /Zc:throwingNew /Zc:inline /Zc:externConstexpr
        /Zc:templateScope /Zc:checkGwOdr /Zc:enumTypes
        /W4
        /wd4459 # local variable name hides global variable
        /wd4702 # unreachable code
        /wd4100 # unreferenced formal parameter
    >
)

#
# All three targets liant/liant_module/liant_module_nostd use basically same headers set
# So factor this set out into the variable
#
set(LIANT_HEADERS
    include/liant/liant.hpp
    include/liant/container.hpp
    include/liant/container_slice.hpp
    include/liant/details/container_slice_impl.hpp
    include/liant/details/container_slice_settings.hpp
    include/liant/details/container_slice_vtable.hpp
    include/liant/details/container_ptr.hpp
    include/liant/container_view.hpp
    include/liant/ptr.hpp
    include/liant/tuple.hpp
    include/liant/typelist.hpp
    include/liant/snake_case.hpp
    include/liant/dependency_macro.hpp
    include/liant/export_macro.hpp
)
set(LIANT_MODULE_HEADERS ${LIANT_HEADERS})
set(LIANT_MODULE_NOSTD_HEADERS ${LIANT_HEADERS})

#
# Conventional, header-based target
#
add_library(liant INTERFACE)
add_library(liant::liant ALIAS liant)
target_compile_features(liant INTERFACE cxx_std_20)
set_target_properties(liant PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    CXX_SCAN_FOR_MODULES OFF
)
target_sources(liant INTERFACE
    FILE_SET liant_headers_set TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES ${LIANT_HEADERS}
)
target_link_libraries(liant INTERFACE liant_warn_lvl)

#
# C++23 module with 'import std'
# Only tested with libc++-20 (libstdc++ don't support 'import std' for now)
#
# Configure this CMake target with '-DCMAKE_CXX_FLAGS="-stdlib=libc++"'
#
add_library(liant_module STATIC)
add_library(liant::liant_module ALIAS liant_module)
target_compile_features(liant_module PUBLIC cxx_std_23)
set_target_properties(liant_module PROPERTIES
    CXX_STANDARD_REQUIRED ON
    # Currently CMake requires extensions enabled when using import std.
    # https://gitlab.kitware.com/cmake/cmake/-/issues/25916
    # https://gitlab.kitware.com/cmake/cmake/-/issues/25539
    CXX_EXTENSIONS ON
    CXX_MODULE_STD ON
)
target_sources(liant_module PUBLIC
    FILE_SET liant_module_headers_set TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES ${LIANT_MODULE_HEADERS}
)
target_sources(liant_module PUBLIC
    FILE_SET liant_module_cxx_modules_set TYPE CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
    FILES
        src/share/liant/liant_std.cppm
)
target_link_libraries(liant_module PRIVATE liant_warn_lvl)
target_compile_definitions(liant_module PRIVATE LIANT_MODULE LIANT_STD_MODULE)

#
# C++20 module with STD in global-module-fragment
#
add_library(liant_module_nostd STATIC)
add_library(liant::liant_module_nostd ALIAS liant_module_nostd)
target_compile_features(liant_module_nostd PUBLIC cxx_std_23)
set_target_properties(liant_module_nostd PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
target_sources(liant_module_nostd PUBLIC
    FILE_SET liant_module_nostd_headers_set TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES ${LIANT_MODULE_NOSTD_HEADERS}
)
target_sources(liant_module_nostd PUBLIC
    FILE_SET liant_module_nostd_cxx_modules_set TYPE CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
    FILES
        src/share/liant/liant_nostd.cppm
)
target_link_libraries(liant_module_nostd PRIVATE liant_warn_lvl)
target_compile_definitions(liant_module_nostd PRIVATE LIANT_MODULE)

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
# Library installation
#
# Use:
#   'cmake --install <build_dir>'                                   install all components
#   'cmake --install <build_dir> --component liant'                 [base installation] install Liant as header-only library (conventional, pre-C++20 library)
#   'cmake --install <build_dir> --component liant_module'          [optional]          install Liant as C++23 module library (C++23 STD module is used here)
#   'cmake --install <build_dir> --conponent liant_module_nostd'    [optional]          install Liant as C++20 module library (standard headers were put into Global Module Fragment)
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

# set target installation location properties and associate it with the targets files
# note:
#   components don't require to have same names as the corresponding targets
#   we've named them this way just for the consistency
#
# 'liant' [header-only] component
install(
    TARGETS
        liant
    EXPORT liant_targets
    FILE_SET liant_headers_set
        COMPONENT liant
)

# 'liant_module' [liant module + std module] component
install(
    TARGETS
        liant_module
    EXPORT liant_targets
    FILE_SET liant_module_headers_set
        COMPONENT liant_module
    FILE_SET liant_module_cxx_modules_set DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT liant_module
    OPTIONAL
)

# 'liant_module_nostd' [liant module + std in global module fragment] component
install(
    TARGETS
        liant_module_nostd
    EXPORT liant_targets
    FILE_SET liant_module_nostd_headers_set
        COMPONENT liant_module_nostd
    FILE_SET liant_module_nostd_cxx_modules_set DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT liant_module_nostd
    OPTIONAL
)
# this one ain't strictly neccesary, it just enables some warnings-as-errors and make MSVC more standard conformant
install(
    TARGETS
        liant_warn_lvl
    EXPORT liant_targets
    COMPONENT liant_warn_lvl
    OPTIONAL
)

# install the targets files (so that you can comsume installed library via 'find_package(...)')
install(
    EXPORT liant_targets
    NAMESPACE liant::
    DESTINATION ${LIANT_INSTALL_CMAKE_DIR}
    COMPONENT liant
)

# install the config and version files
install(
    FILES
        "${PROJECT_BINARY_DIR}/liant-config.cmake"
        "${PROJECT_BINARY_DIR}/liant-config-version.cmake"
    DESTINATION ${LIANT_INSTALL_CMAKE_DIR}
)
