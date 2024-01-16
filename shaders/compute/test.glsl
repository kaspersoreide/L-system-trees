#version 430 core

layout (local_size_x = 1024) in;

layout (std430, binding = 0) buffer block1
{
    float input_data[];
};

layout (std430, binding = 1) buffer block2
{
    float output_data[];
};

void main() {
    uint id = gl_LocalInvocationID.x;
    output_data[id] = input_data[id];
}