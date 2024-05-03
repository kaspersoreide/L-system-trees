#version 430

layout(location = 0) in vec4 pos;

uniform layout(location = 0) mat4 MVP;

layout (binding = 0) coherent readonly buffer block0 
{
    uint lastLeafIdx;
    mat4 Models[];
};

void main() {
    gl_Position = MVP * Models[gl_InstanceID] * pos;
    //gl_Position = MVP * pos;
}