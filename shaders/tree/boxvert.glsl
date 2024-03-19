#version 430

layout(location = 0) in vec4 pos;

uniform layout(location = 0) mat4 Model;
uniform layout(location = 1) mat4 MVP;

out vec3 worldPos;

void main() {
    worldPos = (Model * pos).xyz;
    gl_Position = MVP * pos;
}