#version 430

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 normal;

uniform layout(location = 0) mat4 MVP;

out float brightness;

void main() {
    gl_Position = MVP * pos;
    brightness = max(dot(normal.xyz, vec3(1.0, 0.0, 0.0)), 0.1);
}