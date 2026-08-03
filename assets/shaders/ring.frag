#version 460 core

in float v_radial;
in vec3 v_normal;

uniform vec3 u_light_dir;
uniform sampler2D u_map;
uniform bool u_use_map;
uniform float u_inner_fraction;

out vec4 frag_color;

void main() {
    vec4 texel = vec4(0.75, 0.7, 0.55, 1.0);
    if (u_use_map) {
        // Radial strip: u = distance from inner edge of ring, v fixed mid-row.
        float denom = max(1.0 - u_inner_fraction, 1e-6);
        float u = max(0.0, (v_radial - u_inner_fraction) / denom);
        texel = texture(u_map, vec2(u, 0.5));
    }
    if (texel.a < 0.02) {
        discard;
    }

    vec3 N = normalize(v_normal);
    vec3 L = normalize(u_light_dir);
    float ndotl = max(abs(dot(N, L)), 0.0); // double-sided
    float lit = 0.25 + 0.75 * ndotl;
    frag_color = vec4(texel.rgb * lit, texel.a);
}
