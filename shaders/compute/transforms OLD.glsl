#version 430 core

/*
ideas:
start with 1 thread iterating through the string and write. When a push is found, save the position and find its pop.
next iteration will have 2 threads, 1 from the push and 1 after the pop. 

fff [ ff [ f ] f ] ff
*/

layout (local_size_x = 1) in;

layout (binding = 0) coherent readonly buffer block1 {
    uint string[];
};

layout (binding = 1) coherent readonly buffer block3 {
    uint positions[];
};

layout (binding = 2) coherent writeonly buffer block3 {
    uint next_positions[];
};

layout (binding = 3) coherent buffer block4 {
    mat4 transforms[];
};

layout (location = 0) uniform float turnAngle;
layout (location = 1) uniform float segmentLength;
layout (location = 2) uniform uint stringLength;

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
    next_positions[2 * idx] = 0;
    next_positions[2 * idx + 1] = 0;
    uint i = positions[idx];
    int pushpop = 0;
    mat4 transform = (i > 0) ? transforms[i-1] : mat4(1.0);
    bool writing = 1;
    bool finished = 0;
    while (!finished && i < stringLength) {
        if (writing) {
            transforms[i] = transform;
        }

        uint char = string[i];
        switch (char) {
            case 43:    //+ 
                if (writing) transform *= rotate(turnAngle);
                break;
            case 45:    //-
                if (writing) transform *= rotate(-turnAngle);
                break;
            case 91:    //[
                pushpop++;
                if (pushpop == 1) {
                    next_positions[2 * idx] = i + 1;
                    writing = 0;
                }
                break;
            case 93:    //]
                pushpop--;
                if (pushpop == 0) {
                    next_positions[2 * idx + 1] = i + 1;
                    finished = 1;
                }
                break;
            default:
                if (writing) {
                    vec4 direction = transform * vec4(1.0, 0.0, 0.0, 0.0);
                    transform[3] += segmentLength * direction;
                }
                break;
        };
        i++;
    }
}