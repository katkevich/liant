cmake_minimum_required(VERSION 3.28)

include(FetchContent)

set(BUILD_CURL_EXE OFF)          # Don't build curl executable
set(BUILD_SHARED_LIBS OFF)       # Build static library
set(CURL_STATICLIB ON)           # Enable static library build
# set(HTTP_ONLY ON)                # HTTP/HTTPS only (optional)

# SSL/TLS support - choose one of these options:
# Option 1: Use system OpenSSL (recommended)
# set(CURL_USE_OPENSSL ON)

FetchContent_Declare(curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG 75a2079d5c28debb2eaa848ca9430f1fe0d7844c # curl-8_11_1
    GIT_SHALLOW TRUE
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(curl)

add_executable(wget)
target_compile_features(wget PRIVATE cxx_std_23)
set_target_properties(wget PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
    CXX_MODULE_STD ON
)
target_sources(wget PRIVATE
    "src/main.cpp"
    "src/logger.cpp"
    "src/http_client.cpp"
    "src/cmd_parser.cpp"
    "src/app.cpp"
    "src/help.cpp"
    "src/file_reader.cpp"
    "src/file_writer.cpp"
)

target_sources(wget PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
        "mod/logger.cppm"
        "mod/http_client.cppm"
        "mod/cmd_parser.cppm"
        "mod/app.cppm"
        "mod/wget.cppm"
        "mod/help.cppm"
        "mod/common.cppm"
        "mod/file_reader.cppm"
        "mod/file_writer.cppm"
)
target_link_libraries(wget PUBLIC liant::liant_module CURL::libcurl)

#target_compile_definitions(wget PRIVATE CURL_STATICLIB)
