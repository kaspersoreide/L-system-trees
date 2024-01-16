#version 430

layout(location = 0) in vec4 pos;

uniform layout(location = 0) mat4 MVP;

void main() {
    gl_Position = MVP * pos;
}