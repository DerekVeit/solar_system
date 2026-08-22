#version 460 core

in vec2 v_ndc;

uniform mat4 u_inv_projection;
uniform mat3 u_world_from_view;
uniform mat3 u_tex_from_ecliptic;
uniform sampler2D u_stars;
uniform float u_brightness;

out vec4 frag_color;

const float PI = 3.14159265358979323846;

vec2 equirect_uv(vec3 dir) {
    float lon = atan(dir.y, dir.x);
    float lat = asin(clamp(dir.z, -1.0, 1.0));
    return vec2(lon * (0.5 / PI), 0.5 - lat / PI);
}

void main() {
    vec4 view = u_inv_projection * vec4(v_ndc, 1.0, 1.0);
    vec3 dir_view = normalize(view.xyz / view.w);
    vec3 dir_ecl = normalize(u_world_from_view * dir_view);
    vec3 dir_tex = normalize(u_tex_from_ecliptic * dir_ecl);
    vec3 rgb = texture(u_stars, equirect_uv(dir_tex)).rgb;
    frag_color = vec4(rgb * u_brightness, 1.0);
}
