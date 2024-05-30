#version 430

layout (location = 0) in vec2 pos;

out vec2 u;

void main() {
    u = pos * 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);
}