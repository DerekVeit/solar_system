add_library(solar_core STATIC
    core/body_orientation.cpp
    core/ephemeris.cpp
    core/kepler.cpp
    core/kepler_ephemeris.cpp
    core/json_loader.cpp
    core/sky_orientation.cpp
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

add_library(glad STATIC
    ${CMAKE_SOURCE_DIR}/third_party/glad/src/gl.c
)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/third_party/glad/include)
target_link_libraries(glad PUBLIC ${CMAKE_DL_LIBS})

# stb_image is a header-only library: one .cpp owns STB_IMAGE_IMPLEMENTATION.
add_library(stb STATIC
    ${CMAKE_SOURCE_DIR}/third_party/stb/stb_image.cpp
)
target_include_directories(stb PUBLIC ${CMAKE_SOURCE_DIR}/third_party/stb/include)

add_executable(solar_system
    app/color.cpp
    app/files.cpp
    app/input.cpp
    app/logging.cpp
    app/main.cpp
    app/render/gl_pipeline.cpp
    app/render/gl_renderer.cpp
    app/render/gl_shader.cpp
    app/render/mesh_gen.cpp
    app/run.cpp
    app/scene/body_visual.cpp
    app/scene/body_visual_catalog.cpp
    app/scene/body_visual_geometry.cpp
    app/scene/body_visual_loader.cpp
    app/scene/camera.cpp
    app/scene/scene.cpp
    app/scene/sky_figure_lines.cpp
    app/scene/sky_figure_loader.cpp
    app/scene/sky_loader.cpp
    app/scene/star_catalog_loader.cpp
    app/scene/star_markers.cpp
    app/scene/stroke_font.cpp
    app/scene/texture.cpp
    app/window.cpp
)

find_package(OpenGL REQUIRED)

target_link_libraries(solar_system
    PRIVATE
        solar_core
        glad
        stb
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
