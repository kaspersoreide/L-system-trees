#version 430 core

/*
ideas:
start with 1 thread iterating through the string and write. When a push is found, save the position and find its pop.
next iteration will have 2 threads, 1 from the push and 1 after the pop. 

F, F, F, ]


*/

layout (local_size_x = 1) in;

layout (binding = 0) coherent readonly buffer block1
{
    uint string[];
};

layout (binding = 1) coherent readonly buffer block2
{
    mat4 transforms[];
};

layout (binding = 2) coherent writeonly buffer block3
{
    vec4 vertices[];    //line segments
};

mat4 rotate(float a) {
    return mat3(
        cos(a), -sin(a), 0.0, 0.0,
        sin(a), cos(a), 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

void main() {
    uint idx = gl_GlobalInvocationID.x;
    uint char = string[idx];
    switch (char) {
        case 43:    //+ 
            break;
        case 45:    //-
            break;
        case 91:    //[
            break;
        case 93:    //]
            break;
        default:
            vertices[2 * idx] = transforms[idx][3];
            vertices[2 * idx + 1] = transforms[idx + 1][3];
            break;
    };
}