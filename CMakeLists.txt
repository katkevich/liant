cmake_minimum_required(VERSION 3.28)

project(liant)

add_library(liant INTERFACE)
target_sources(liant INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/liant/liant.hpp
        include/liant/container.hpp
        include/liant/dependency.hpp
        include/liant/ptr.hpp
        include/liant/tuple.hpp
        include/liant/typelist.hpp
        include/liant/snake_case.hpp
)
target_compile_features(liant INTERFACE cxx_std_20)
target_compile_options(liant INTERFACE
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
set_target_properties(liant PROPERTIES CXX_STANDARD_REQUIRED ON)

#
# Tests
#
option(LIANT_BUILD_TESTS "Build Liant tests" ${PROJECT_IS_TOP_LEVEL})

if (LIANT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

#
# Library installation
#
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(LIANT_INSTALL_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/liant)

# generate version file
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/liant-version.cmake"
    VERSION 1.0.0
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
        "${PROJECT_BINARY_DIR}/liant-version.cmake"
    DESTINATION ${LIANT_INSTALL_CMAKE_DIR}
)
