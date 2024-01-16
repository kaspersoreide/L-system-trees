//this is a modified version of https://github.com/openglsuperbible/sb6code/blob/master/bin/media/shaders/prefixsum/prefixsum.cs.glsl

/*
this takes an input buffer that is a long string. but strings don't exist in glsl,
so a buffer of uints will do. The input character will have an output string.
the shader finds the index into the productions array and the string size.
it then uses prefix-sum to calculate the final offset and size of every production in
the final output. 
*/

#version 430 core

layout (local_size_x = 1024) in;

layout (binding = 0) coherent readonly buffer block1
{
    uvec2 input_data[gl_WorkGroupSize.x];
};

layout (binding = 1) coherent writeonly buffer block2
{
    uvec2 output_data[gl_WorkGroupSize.x];
};

shared uint shared_data[gl_WorkGroupSize.x * 2];

void main(void)
{
    uint id = gl_GlobalInvocationID.x;
    uint rd_id;
    uint wr_id;
    uint mask;

    const uint steps = uint(log2(gl_WorkGroupSize.x)) + 1;
    uint step = 0;

    shared_data[id * 2] = input_data[id * 2].y;
    shared_data[id * 2 + 1] = input_data[id * 2 + 1].y;

    barrier();

    for (step = 0; step < steps; step++)
    {
        mask = (1 << step) - 1;
        rd_id = ((id >> step) << (step + 1)) + mask;
        wr_id = rd_id + 1 + (id & mask);
 
        shared_data[wr_id] += shared_data[rd_id];

        barrier();
    }
    output_data[id * 2].x = input_data[id * 2].x;
    output_data[id * 2 + 1].x = input_data[id * 2 + 1].x;

    output_data[id * 2].y = (id > 0) ? shared_data[id * 2 - 1] : 0;
    output_data[id * 2 + 1].y = shared_data[id * 2];
}