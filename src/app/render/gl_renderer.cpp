#include "app/render/gl_renderer.hpp"

#include "app/render/gl_shader.hpp"
#include "app/render/types.hpp"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>

#include <cmath>
#include <cstddef>
#include <numbers>
#include <string_view>
#include <vector>

namespace solar::app {

namespace {

constexpr float kPi = std::numbers::pi_v<float>;

constexpr std::string_view kSphereVertexShader = R"(#version 460 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec2 v_uv;
out vec3 v_normal;

void main() {
    v_uv = a_uv;
    const float PI = 3.14159265;
    v_normal = normalize(mat3(u_model) * a_pos);
    gl_Position = u_projection * u_view * u_model * vec4(a_pos, 1.0);
}
)";

constexpr std::string_view kSphereFragmentShader = R"(#version 460 core
in vec2 v_uv;
in vec3 v_normal;

uniform vec4 u_color;
uniform float u_ambient;
uniform float u_emission;
uniform vec3 u_light_dir;
uniform sampler2D u_diffuse;
uniform bool u_use_diffuse;

out vec4 frag_color;

void main() {
    vec3 N = normalize(v_normal);
    vec3 L = normalize(u_light_dir);

    float ndotl = max(dot(N, L), 0.0);

    vec3 albedo = u_color.rgb;
    if (u_use_diffuse) {
        albedo *= texture(u_diffuse, v_uv).rgb;
    }
    vec3 lit = albedo * (u_ambient + (1.0 - u_ambient) * ndotl);
    vec3 rgb = mix(lit, albedo, u_emission);

    frag_color = vec4(rgb, u_color.a);
}
)";

constexpr std::string_view kLineVertexShader = R"(#version 460 core
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec4 a_color;

uniform mat4 u_view;
uniform mat4 u_projection;

out vec4 v_color;

void main() {
    gl_Position = u_projection * u_view * vec4(a_pos, 1.0);
    v_color = a_color;
}
)";

constexpr std::string_view kLineFragmentShader = R"(#version 460 core
in vec4 v_color;
out vec4 frag_color;
void main() {
    frag_color = v_color;
}
)";

// Low-poly unit sphere: enough to read as a ball at solar-system zoom levels.
constexpr int kSphereSlices = 48;
constexpr int kSphereStacks = 24;

void generate_unit_sphere(std::vector<float>& positions, std::vector<unsigned int>& indices) {
    positions.clear();
    indices.clear();

    // Body axial orientation and rotation are not yet represented.
    // u = longitude 0..1, v = latitude 0..1 (from top to bottom)
    for (int stack = 0; stack <= kSphereStacks; ++stack) {
        const float v = static_cast<float>(stack) / static_cast<float>(kSphereStacks);
        const float phi = v * kPi;
        const float z = std::cos(phi);
        const float ring_radius = std::sin(phi);

        for (int slice = 0; slice <= kSphereSlices; ++slice) {
            const float u = static_cast<float>(slice) / static_cast<float>(kSphereSlices);
            const float theta = u * (2.0f * kPi);
            const float x = ring_radius * std::cos(theta);
            const float y = ring_radius * std::sin(theta);
            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
            positions.push_back(u);
            positions.push_back(v);
        }
    }

    const int stride = kSphereSlices + 1;
    for (int stack = 0; stack < kSphereStacks; ++stack) {
        for (int slice = 0; slice < kSphereSlices; ++slice) {
            const unsigned int i0 = static_cast<unsigned int>(stack * stride + slice);
            const unsigned int i1 = i0 + static_cast<unsigned int>(stride);
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i0 + 1);
            indices.push_back(i0 + 1);
            indices.push_back(i1);
            indices.push_back(i1 + 1);
        }
    }
}

void setup_line_vertex_layout() {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex),
                          reinterpret_cast<void*>(offsetof(LineVertex, x_km)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(LineVertex),
                          reinterpret_cast<void*>(offsetof(LineVertex, color)));
}

} // namespace

GlRenderer::~GlRenderer() { destroy(); }

void GlRenderer::destroy() {
    if (sphere_program_ != 0) {
        glDeleteProgram(sphere_program_);
        sphere_program_ = 0;
    }
    if (line_program_ != 0) {
        glDeleteProgram(line_program_);
        line_program_ = 0;
    }
    if (sphere_vbo_ != 0) {
        glDeleteBuffers(1, &sphere_vbo_);
        sphere_vbo_ = 0;
    }
    if (sphere_ebo_ != 0) {
        glDeleteBuffers(1, &sphere_ebo_);
        sphere_ebo_ = 0;
    }
    if (sphere_vao_ != 0) {
        glDeleteVertexArrays(1, &sphere_vao_);
        sphere_vao_ = 0;
    }
    if (line_vbo_ != 0) {
        glDeleteBuffers(1, &line_vbo_);
        line_vbo_ = 0;
    }
    if (line_vao_ != 0) {
        glDeleteVertexArrays(1, &line_vao_);
        line_vao_ = 0;
    }
    sphere_view_loc_ = -1;
    sphere_projection_loc_ = -1;
    sphere_model_loc_ = -1;
    sphere_color_loc_ = -1;
    sphere_ambient_loc_ = -1;
    sphere_emission_loc_ = -1;
    sphere_light_dir_loc_ = -1;
    line_view_loc_ = -1;
    line_projection_loc_ = -1;
    sphere_index_count_ = 0;
    max_spheres_ = 0;
    max_line_vertices_ = 0;
    max_line_trail_vertices_ = 0;
    max_line_loop_vertices_ = 0;
    for (auto item : textures_) {
        glDeleteTextures(1, &item.second);
    }
    textures_.clear();
}

bool GlRenderer::init(const RenderCapacity& capacity) {
    if (capacity.max_spheres == 0) {
        return false;
    }

    destroy();

    try {
        // % = potential extraction soon

        // % sphere_program_ = create_program(kSphereVertexShader, kSphereFragmentShader)
        const unsigned int sphere_vertex_shader =
            compile_shader(GL_VERTEX_SHADER, kSphereVertexShader);
        const unsigned int sphere_fragment_shader =
            compile_shader(GL_FRAGMENT_SHADER, kSphereFragmentShader);
        sphere_program_ = link_program(sphere_vertex_shader, sphere_fragment_shader);
        glDeleteShader(sphere_vertex_shader);
        glDeleteShader(sphere_fragment_shader);

        // % line_program_ = create_program(kLineVertexShader, kLineFragmentShader)
        const unsigned int line_vertex_shader = compile_shader(GL_VERTEX_SHADER, kLineVertexShader);
        const unsigned int line_fragment_shader =
            compile_shader(GL_FRAGMENT_SHADER, kLineFragmentShader);
        line_program_ = link_program(line_vertex_shader, line_fragment_shader);
        glDeleteShader(line_vertex_shader);
        glDeleteShader(line_fragment_shader);

        // % cache_sphere_uniforms()
        sphere_view_loc_ = glGetUniformLocation(sphere_program_, "u_view");
        sphere_projection_loc_ = glGetUniformLocation(sphere_program_, "u_projection");
        sphere_model_loc_ = glGetUniformLocation(sphere_program_, "u_model");
        sphere_color_loc_ = glGetUniformLocation(sphere_program_, "u_color");
        sphere_ambient_loc_ = glGetUniformLocation(sphere_program_, "u_ambient");
        sphere_emission_loc_ = glGetUniformLocation(sphere_program_, "u_emission");
        sphere_light_dir_loc_ = glGetUniformLocation(sphere_program_, "u_light_dir");
        sphere_diffuse_loc_ = glGetUniformLocation(sphere_program_, "u_diffuse");
        sphere_use_diffuse_loc_ = glGetUniformLocation(sphere_program_, "u_use_diffuse");

        // % cache_line_uniforms()
        line_view_loc_ = glGetUniformLocation(line_program_, "u_view");
        line_projection_loc_ = glGetUniformLocation(line_program_, "u_projection");

        // % create_sphere_mesh_gpu()
        std::vector<float> sphere_positions;
        std::vector<unsigned int> sphere_indices;
        generate_unit_sphere(sphere_positions, sphere_indices);
        sphere_index_count_ = static_cast<int>(sphere_indices.size());

        glGenVertexArrays(1, &sphere_vao_);
        glGenBuffers(1, &sphere_vbo_);
        glGenBuffers(1, &sphere_ebo_);
        glBindVertexArray(sphere_vao_);
        glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo_);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(sphere_positions.size() * sizeof(float)),
                     sphere_positions.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(sphere_indices.size() * sizeof(unsigned int)),
                     sphere_indices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                              reinterpret_cast<void*>(0));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                              reinterpret_cast<void*>(3 * sizeof(float)));

        // % create_line_buffers(capacity)
        glGenVertexArrays(1, &line_vao_);
        glGenBuffers(1, &line_vbo_);
        glBindVertexArray(line_vao_);
        glBindBuffer(GL_ARRAY_BUFFER, line_vbo_);
        const std::size_t line_buffer_capacity = capacity.max_line_vertices +
                                                 capacity.max_line_trail_vertices +
                                                 capacity.max_line_loop_vertices;
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(line_buffer_capacity * sizeof(LineVertex)), nullptr,
                     GL_DYNAMIC_DRAW);
        setup_line_vertex_layout();

        glBindVertexArray(0);

        max_spheres_ = capacity.max_spheres;
        max_line_vertices_ = capacity.max_line_vertices;
        max_line_trail_vertices_ = capacity.max_line_trail_vertices;
        max_line_loop_vertices_ = capacity.max_line_loop_vertices;
        return true;
    } catch (const std::exception&) {
        destroy();
        return false;
    }
}

void GlRenderer::set_camera_uniforms(unsigned int program, int view_loc, int projection_loc,
                                     const glm::mat4& view, const glm::mat4& projection) const {
    glUseProgram(program);
    if (view_loc >= 0) {
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
    }
    if (projection_loc >= 0) {
        glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));
    }
}

void GlRenderer::draw_spheres(std::span<const SphereInstance> spheres, const glm::mat4& view,
                              const glm::mat4& projection) {
    if (sphere_program_ == 0 || sphere_vao_ == 0 || sphere_index_count_ <= 0) {
        return;
    }

    const std::size_t count = spheres.size() < max_spheres_ ? spheres.size() : max_spheres_;
    if (count == 0) {
        return;
    }

    set_camera_uniforms(sphere_program_, sphere_view_loc_, sphere_projection_loc_, view,
                        projection);
    glBindVertexArray(sphere_vao_);

    for (std::size_t i = 0; i < count; ++i) {
        const SphereInstance& sphere = spheres[i];
        if (sphere.radius_km <= 0.0f) {
            continue;
        }

        const BodySurface& surface = sphere.surface;

        const glm::mat4 model =
            glm::translate(glm::mat4{1.0f}, glm::vec3{sphere.x_km, sphere.y_km, sphere.z_km}) *
            glm::scale(glm::mat4{1.0f}, glm::vec3{sphere.radius_km});

        if (sphere_model_loc_ >= 0) {
            glUniformMatrix4fv(sphere_model_loc_, 1, GL_FALSE, glm::value_ptr(model));
        }

        const bool using_diffuse_texture =
            (!surface.textures.diffuse.empty() && textures_.contains(surface.textures.diffuse) &&
             sphere_diffuse_loc_ >= 0 && sphere_use_diffuse_loc_ >= 0);

        if (sphere_color_loc_ >= 0) {
            if (using_diffuse_texture) {
                glUniform4f(sphere_color_loc_, 1.0f, 1.0f, 1.0f, surface.color.a);
            } else {
                glUniform4f(sphere_color_loc_, surface.color.r, surface.color.g, surface.color.b,
                            surface.color.a);
            }
        }
        if (using_diffuse_texture) {
            auto diffuse_id = textures_[surface.textures.diffuse];
            glUniform1i(sphere_use_diffuse_loc_, true);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuse_id);
            glUniform1i(sphere_diffuse_loc_, 0);
        } else {
            glUniform1i(sphere_use_diffuse_loc_, false);
        }

        if (sphere_ambient_loc_ >= 0) {
            glUniform1f(sphere_ambient_loc_, surface.ambient);
        }
        if (sphere_emission_loc_ >= 0) {
            glUniform1f(sphere_emission_loc_, surface.emission);
        }
        if (sphere_light_dir_loc_ >= 0) {
            glUniform3fv(sphere_light_dir_loc_, 1, glm::value_ptr(sphere.light_dir));
        }
        glDrawElements(GL_TRIANGLES, sphere_index_count_, GL_UNSIGNED_INT, nullptr);
    }
}

void GlRenderer::draw_line_primitives(unsigned int mode, std::span<const LinePrimitive> primitives,
                                      std::size_t max_vertices, const glm::mat4& view,
                                      const glm::mat4& projection) {
    if (line_program_ == 0 || line_vao_ == 0 || line_vbo_ == 0) {
        return;
    }

    set_camera_uniforms(line_program_, line_view_loc_, line_projection_loc_, view, projection);
    glBindVertexArray(line_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, line_vbo_);

    for (const LinePrimitive& primitive : primitives) {
        const std::size_t count =
            primitive.vertices.size() < max_vertices ? primitive.vertices.size() : max_vertices;
        if (count < 2) {
            continue;
        }

        glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(count * sizeof(LineVertex)),
                        primitive.vertices.data());
        glDrawArrays(mode, 0, static_cast<GLsizei>(count));
    }
}

void GlRenderer::draw(const DrawBatch& batch, const glm::mat4& view, const glm::mat4& projection) {
    draw_spheres(batch.spheres, view, projection);
    draw_line_primitives(GL_LINE_STRIP, batch.line_trails, max_line_trail_vertices_, view,
                         projection);
    draw_line_primitives(GL_LINE_LOOP, batch.line_loops, max_line_loop_vertices_, view, projection);
    draw_line_primitives(GL_LINES, batch.lines, max_line_vertices_, view, projection);
}

void GlRenderer::upload_texture(const std::string& path, const TextureImage& image) {
    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width, image.height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, image.pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (textures_.contains(path)) {
        glDeleteTextures(1, &textures_[path]);
    }
    textures_[path] = id;
}

} // namespace solar::app
