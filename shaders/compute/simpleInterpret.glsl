#version 430 core

layout (local_size_x = 1) in;

layout (binding = 0) coherent readonly buffer block0
{
    uint string[];
};

struct Node {
    uint idx;
    uint parent;
    uvec4 children;
    mat4 transform;
    float width;
}

layout (binding = 1) coherent writeonly buffer block1
{
    uint lastIdx;   //index of last node in buffer
    float x_min;
    float x_max;
    float y_min;
    float y_max;
    float z_min;
    float z_max;
    Node tree[];   
};

/*
layout (binding = 1) coherent writeonly buffer block3
{
    int numLeaves;
    mat4 leafModels[];   
};
*/

uniform layout (location = 0) uint stringLength;
uniform layout (location = 1) float branchWidth;
uniform layout (location = 2) float turnAngle;
uniform layout (location = 3) int cylinderSegments;

// found at https://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
// rotates counterclockwise hopefully
mat4 rotationMatrix(vec3 axis, float angle) {
    vec3 a = normalize(axis);

    float s = -sin(angle);
    float c = cos(angle);

    float oc = 1.0 - c;

    float sx = s * a.x;
    float sy = s * a.y;
    float sz = s * a.z;

    float ocx = oc * a.x;
    float ocy = oc * a.y;
    float ocz = oc * a.z;

    float ocxx = ocx * a.x;
    float ocxy = ocx * a.y;
    float ocxz = ocx * a.z;
    float ocyy = ocy * a.y;
    float ocyz = ocy * a.z;
    float oczz = ocz * a.z;

    return mat4(
        vec4(ocxx + c, ocxy - sz, ocxz + sy, 0.0),
        vec4(ocxy + sz, ocyy + c, ocyz - sx, 0.0),
        vec4(ocxz - sy, ocyz + sx, oczz + c, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
}

mat4 translate(vec3 v) {
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(v, 1.0)
    );
}

const float PI = 3.14159265358979323846;

void main() {
    int top = -1;
    uint stack[16];
    uint idx = 0;
    //set initial state pointing upwards (y-axis), because the y axis is always up and trees grow upwards
    tree[idx].transform = mat4(
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
    tree[idx].width = branchWidth;
    tree[idx].parent = 0;

    for (uint i = 0; i < stringLength; i++) {
        switch (string[i]) {
            case 43:    // + turn left
                tree[idx].transform = tree[idx].transform * rotationMatrix(vec3(0.0, 0.0, 1.0), turnAngle);
                break;
            case 45:    // - turn right
                tree[idx].transform = tree[idx].transform * rotationMatrix(vec3(0.0, 0.0, 1.0), -turnAngle);
                break;
            case 38:    // & pitch down
                tree[idx].transform = tree[idx].transform * rotationMatrix(vec3(0.0, 1.0, 0.0), turnAngle);
                break;
            case 94:    // ^ pitch up
                tree[idx].transform = tree[idx].transform * rotationMatrix(vec3(0.0, 1.0, 0.0), -turnAngle);
                break;
            case 92:    // \ roll left
                tree[idx].transform = tree[idx].transform * rotationMatrix(vec3(1.0, 0.0, 0.0), turnAngle);
                break;
            case 47:    // / roll right
                tree[idx].transform = tree[idx].transform * rotationMatrix(vec3(1.0, 0.0, 0.0), -turnAngle);
                break;
            case 91:    // [ push state
                stack[++top] = idx;              
                break;
            case 93:    // ] pop state
                idx = stateStack[top--];
                break;
            case 33:    // ! decrement segment width
                tree[idx].width *= 0.8;
                break;
            case 76:    // L (BIG L) make leaf

                break;
            default:    // go forward, add new node to tree, connect to parent
                //check child list for available entry
                //create child node, set parent and other data
                //increment lastIdx
                
                break;
        };
    }
}