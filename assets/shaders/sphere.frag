#version 460 core

in vec2 v_uv;
in vec3 v_normal;

uniform vec4 u_color;
uniform float u_ambient;
uniform float u_emission;
uniform vec3 u_light_dir;
uniform float u_texture_offset;
uniform sampler2D u_diffuse;
uniform sampler2D u_night;
uniform bool u_use_diffuse;
uniform bool u_use_night;
uniform bool u_show_graticules;

out vec4 frag_color;

float proximity_to_cycle(float fraction, float scale, float target, float limit, float cycle) {
    float actual = fraction * scale;
    float half_scale = scale / 2.0;
    float offset = half_scale - target;
    float distance = 0.0;
    if (cycle > 0.0) {
        distance = abs(mod(half_scale, cycle) - mod(actual + offset, cycle));
    } else {
        distance = abs(half_scale - (actual + offset));
    }
    if (distance > limit) return 0.0;
    return 1.0 - distance / limit;
}

void main() {
    // The mesh and `v_uv` are considered aligned with the geographic coordinate system.
    // The `texture_uv` is for aligning the texture map 0° to the mesh 0°.

    vec2 texture_uv = vec2(v_uv.x + u_texture_offset / 360, v_uv.y);

    vec3 N = normalize(v_normal);
    vec3 L = normalize(u_light_dir);

    float ndotl = dot(N, L);
    float sun_facing = max(ndotl, 0.0);
    float sun_long_deg = degrees(asin(ndotl));

    vec3 day_rgb = u_color.rgb;
    vec3 night_rgb = u_ambient * day_rgb;
    if (u_use_diffuse) {
        day_rgb = texture(u_diffuse, texture_uv).rgb;
        night_rgb = u_ambient * day_rgb;
        if (u_use_night) {
            night_rgb = texture(u_night, texture_uv).rgb;
        }
    }
    vec3 unlit = day_rgb * u_ambient;
    vec3 day_lit = mix(unlit, day_rgb, sun_facing);

    float nightness = 0;
    float offset = 10.0;
    if (-90.0 < sun_long_deg && sun_long_deg < -offset) {
        nightness = 1.0;
    } else if (-offset < sun_long_deg && sun_long_deg < offset) {
        nightness = (offset - sun_long_deg) / (2 * offset);
    }
    vec3 night_lit = mix(unlit, night_rgb, nightness);

    vec3 lit = mix(day_lit, night_lit, nightness);

    vec3 rgb = mix(lit, day_rgb, u_emission);

    vec3 red = {1.0, 0.0, 0.0};
    vec3 white = {1.0, 1.0, 1.0};
    vec3 gray = {0.5, 0.5, 0.5};
    vec3 blue = {0.0, 0.0, 1.0};
    vec3 cyan = {0.0, 1.0, 1.0};

    if (u_show_graticules) {
        rgb = mix(rgb, white, proximity_to_cycle(v_uv.x, 360.0, 90.0, 0.5, 180.0) / 4);
        rgb = mix(rgb, blue, proximity_to_cycle(v_uv.x, 360.0, 180.0, 0.5, 360.0) / 4);
        rgb = mix(rgb, red, proximity_to_cycle(v_uv.x, 360.0, 0.0, 0.5, 360.0) / 4);

        rgb = mix(rgb, gray, proximity_to_cycle(v_uv.x, 360.0, 0.0, 0.5, 15.0) / 24);
        rgb = mix(rgb, gray, proximity_to_cycle(v_uv.y, 180.0, 0.0, 0.5, 10.0) / 24);

        rgb = mix(rgb, red, proximity_to_cycle(v_uv.y, 180.0, 90.0, 0.5, 0.0) / 4);

        rgb = mix(rgb, cyan, proximity_to_cycle(abs(dot(N, L)), 1.0, 0.0, 0.005, 0.0) / 4);
    }

    frag_color = vec4(rgb, u_color.a);
}
