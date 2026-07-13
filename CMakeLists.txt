cmake_minimum_required(VERSION 3.22)

project(solar_system
    VERSION 0.1.0
    DESCRIPTION "Solar system geometry and motion simulation"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SOLAR_SYSTEM_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(SOLAR_SYSTEM_WARNINGS_AS_ERRORS)
        add_compile_options(-Werror)
    endif()
endif()

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.0.2
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(fmt glm nlohmann_json Catch2)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW3 REQUIRED glfw3)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)