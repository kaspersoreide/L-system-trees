#version 430 core

#define N_PRODUCTIONS 7

layout (local_size_x = 32) in;

layout (binding = 0) coherent readonly buffer block1
{
    uvec2 input_data[];
};

layout (binding = 1) coherent writeonly buffer block2
{
    uint output_data[];
};

layout (binding = 2) coherent readonly buffer productions
{
    uint inputs[N_PRODUCTIONS];
    uint offsets[N_PRODUCTIONS];
    uint lengths[N_PRODUCTIONS];
    float probabilities[N_PRODUCTIONS];
    uint string[];
};

void main() {
    uint id = gl_GlobalInvocationID.x;
    uint prodIdx = input_data[id].x;
    uint stringLength = lengths[prodIdx];
    for (uint i = 0; i < stringLength; i++) {
        uint outIdx = input_data[id].y + i;
        output_data[outIdx] = string[offsets[prodIdx] + i];
    }
}