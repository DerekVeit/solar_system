#version 460 core

const vec2 kVerts[3] = vec2[](vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));

out vec2 v_ndc;

void main() {
    vec2 p = kVerts[gl_VertexID];
    v_ndc = p;
    gl_Position = vec4(p, 1.0, 1.0);
}
