cmake_minimum_required(VERSION 4.0.3)

project(liant VERSION 1.0.0)

#
# common settings for liant/liant-mod targets
#
add_library(liant-common INTERFACE)
target_compile_options(liant-common INTERFACE
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
set_target_properties(liant PROPERTIES CXX_STANDARD_REQUIRED ON)
target_link_libraries(liant INTERFACE liant-common)

#
# C++20 modules based target
#
add_library(liant-mod STATIC)
target_compile_features(liant-mod PUBLIC cxx_std_23)
set_target_properties(liant-mod PROPERTIES CXX_STANDARD_REQUIRED ON)
target_sources(liant-mod PRIVATE
    FILE_SET HEADERS
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
target_sources(liant-mod PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
    FILES
        src/liant.ixx # primary module-interface-unit
)
target_link_libraries(liant-mod PRIVATE liant-common)
target_compile_definitions(liant-mod PRIVATE LIANT_MODULE)


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
# Library installation (only header-base library can be installed)
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
    TARGETS liant-common
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
