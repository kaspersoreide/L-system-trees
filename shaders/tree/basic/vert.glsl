#version 430


layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 normal;

uniform layout(location = 0) mat4 Model;
uniform layout(location = 1) mat4 MVP;
uniform layout(location = 3) mat4 VP;

out vec3 rotatedNormal;
out vec3 rawPos;


void main() {
    rotatedNormal = (Model * normal).xyz;
    rawPos = pos.xyz;
    gl_Position = MVP * vec4(pos.xyz, 1.0);
}