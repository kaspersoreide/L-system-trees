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

void main() {
    //skip the first work group as only the other ones need to add the work group sums to
    uint g_id = gl_WorkGroupID.x + 1;
    uint id = g_id * gl_WorkGroupSize.x + gl_LocalInvocationID.x;
    
    uint sum = 0;
    for (uint i = 1; i < g_id; i++) {
        //read last entry of every workgroup and add to sum
        uint r_id = i * gl_WorkGroupSize.x - 1;
        sum += input_data[r_id].y;
    }
    output_data[id].y = input_data[id].y + sum;
}