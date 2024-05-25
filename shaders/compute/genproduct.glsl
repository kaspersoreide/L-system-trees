#version 430 core

layout (local_size_x = 32) in;

layout (binding = 0) coherent readonly buffer block1
{
    uvec2 input_data[];
};

layout (binding = 1) coherent writeonly buffer block2
{
    uint output_data[];
};

struct Production {
    uint predecessor;
    float proability;
    uint size;
    uint successor[64]; 
};

layout (binding = 2) coherent readonly buffer block3
{
    uint n_productions;
    Production productions[];
};

void main() {
    uint id = gl_GlobalInvocationID.x;
    uint prodIdx = input_data[id].x;
    uint stringLength = productions[prodIdx].size;
    for (uint i = 0; i < stringLength; i++) {
        uint outIdx = input_data[id].y + i;
        output_data[outIdx] = productions[prodIdx].successor[i];
    }
}