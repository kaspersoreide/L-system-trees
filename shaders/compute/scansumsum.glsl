#version 430 core

layout (local_size_x = 1024) in;

layout (binding = 0) coherent readonly buffer block1
{
    uvec2 input_data[];
};

layout (binding = 1) coherent writeonly buffer block2
{
    uvec2 output_data[];
};

struct Production {
    uint predecessor;
    float probability;
    uint size;
    uint successor[64]; 
};

layout (binding = 2) coherent readonly buffer block3
{
    uint n_productions;
    Production productions[];
};

void main() {
    uint g_id = gl_WorkGroupID.x;
    uint sum = 0;
    for (uint i = 1; i <= g_id; i++) {
        //read last entry of every workgroup and add to sum
        uint r_id = i * gl_WorkGroupSize.x - 1;
        uint p_id = input_data[r_id].x;
        sum += input_data[r_id].y + productions[p_id].size;
    }
    uint idx = gl_GlobalInvocationID.x;
    //output_data[idx].x = input_data[idx].x;
    output_data[idx].y = input_data[idx].y + sum;
}