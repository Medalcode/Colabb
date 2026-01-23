cmake_minimum_required(VERSION 3.16)
project(colabb VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(VTE REQUIRED vte-2.91)
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(SECRET REQUIRED libsecret-1)

# nlohmann/json (header-only)
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Source files
set(SOURCES
    src/main.cpp
    src/infrastructure/terminal/vte_terminal.cpp
    src/infrastructure/http/http_client.cpp
    src/infrastructure/config/config_manager.cpp
    src/domain/ai/groq_provider.cpp
    src/domain/ai/openai_provider.cpp
    src/application/prediction_service.cpp
    src/application/suggestion_cache.cpp
    src/ui/main_window.cpp
    src/ui/config_dialog.cpp
    src/ui/search_bar.cpp
    src/ui/tab_manager.cpp
)

add_executable(colabb ${SOURCES})

target_include_directories(colabb PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${GTK3_INCLUDE_DIRS}
    ${VTE_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${SECRET_INCLUDE_DIRS}
)

target_link_libraries(colabb
    ${GTK3_LIBRARIES}
    ${VTE_LIBRARIES}
    ${CURL_LIBRARIES}
    ${SECRET_LIBRARIES}
    nlohmann_json::nlohmann_json
    pthread
)

# Install
install(TARGETS colabb DESTINATION bin)

# Testing (optional)
option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
