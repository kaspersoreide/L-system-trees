#version 430 core
layout (local_size_x = 1) in;

layout (binding = 0) coherent readonly buffer block1 {
    uint string[];
};

layout (binding = 1) coherent writeonly buffer block2 {
    // data is structured like this:
    // x: offset of [ into string buffer
    // y: offset of corresponding ]
    uvec2 level0[];
    uvec2 le
};

uniform layout (location = 0) uint stringLength;

void main() {
    //make 1 bracket for every "gap" between brackets too. 
    //for example the string "f f f [ f ] f [ f ] f" will have 3 "brackets" at the root level
    int depth = 0;
    for (int i = 0; i < stringLength; i++) {
        switch (string[i]) {
            case 91:    //[

                break;
            case 93:    //]

                break;
            default:
                break;
        };
    }
}