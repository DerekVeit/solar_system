add_library(solar_core STATIC
    core/kepler.cpp
    core/kepler_ephemeris.cpp
    core/json_loader.cpp
    sim/clock.cpp
    sim/solar_system.cpp
)

target_include_directories(solar_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(solar_core
    PUBLIC
        fmt::fmt
        glm::glm
        nlohmann_json::nlohmann_json
)

add_executable(solar_system
    app/color.cpp
    app/files.cpp
    app/input.cpp
    app/logging.cpp
    app/main.cpp
    app/window.cpp
)

find_package(OpenGL REQUIRED)

target_link_libraries(solar_system
    PRIVATE
        solar_core
        OpenGL::GL
        ${GLFW3_LIBRARIES}
)

target_include_directories(solar_system
    PRIVATE
        ${GLFW3_INCLUDE_DIRS}
)

target_compile_options(solar_system
    PRIVATE
        ${GLFW3_CFLAGS_OTHER}
)

target_link_options(solar_system
    PRIVATE
        ${GLFW3_LDFLAGS}
)

add_custom_command(TARGET solar_system POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:solar_system>/assets
    COMMENT "Copy assets next to solar_system executable"
)