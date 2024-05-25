#version 430

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 texCoords;

uniform layout(location = 0) mat4 MVP;

layout (binding = 0) coherent readonly buffer block0 
{
    uint lastLeafIdx;
    mat4 Models[];
};

out vec2 uv;
out vec3 normal;

void main() {
    float leafScale = 0.8;
    vec4 scaledPos = vec4(leafScale * pos.xyz, 1.0);
    gl_Position = MVP * Models[gl_InstanceID] * scaledPos;
    uv = texCoords;
    normal = (Models[gl_InstanceID] * vec4(0.0, 1.0, 0.0, 0.0)).xyz;
    //gl_Position = MVP * pos;
}