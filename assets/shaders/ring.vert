#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in float a_radial;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out float v_radial;
out vec3 v_normal;

void main() {
    v_radial = a_radial;
    // Body +Z is the ring plane normal (matches sphere body frame).
    v_normal = normalize(mat3(u_model) * vec3(0.0, 0.0, 1.0));
    gl_Position = u_projection * u_view * u_model * vec4(a_pos, 1.0);
}
