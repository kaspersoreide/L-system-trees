#version 430 core

layout (local_size_x = 1) in;

layout (binding = 0) coherent readonly buffer block0
{
    uint string[];
};

struct Node {
    uint idx;
    uint parent;
    uvec4 children;     //max 4 children
    vec4 pos;
    float width;
};

layout (binding = 1) coherent writeonly buffer block1
{
    uint lastIdx;   //index of last node in buffer
    Node tree[];   
};

layout (binding = 2) coherent writeonly buffer block2
{
    vec4 boxVertices[36];   //6 vertices per face, 6 faces
}

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

struct State {
    mat4 transform;
    float width;
    uint treeIdx;
}

void main() {
    vec3 boxMin = vec3(0);
    vec3 boxMax = vec3(0);
    int top = -1;
    State stack[16];
    uint idx = 0;
    State currentState;
    //set initial state pointing upwards (y-axis), because the y axis is always up and trees grow upwards
    currentState.transform = mat4(
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
    currentState.treeIdx = idx;
    currentState.width = branchWidth;
    tree[idx].pos = vec4(0.0, 0.0, 0.0, 1.0);
    tree[idx].width = branchWidth;
    tree[idx].parent = 0;
    tree[idx].children = uvec4(0);
    lastIdx = 0;
    for (uint i = 0; i < stringLength; i++) {
        switch (string[i]) {
            case 43:    // + turn left
                currentState.transform = currentState.transform * rotationMatrix(vec3(0.0, 0.0, 1.0), turnAngle);
                break;
            case 45:    // - turn right
                currentState.transform = currentState.transform * rotationMatrix(vec3(0.0, 0.0, 1.0), -turnAngle);
                break;
            case 38:    // & pitch down
                currentState.transform = currentState.transform * rotationMatrix(vec3(0.0, 1.0, 0.0), turnAngle);
                break;
            case 94:    // ^ pitch up
                currentState.transform = currentState.transform * rotationMatrix(vec3(0.0, 1.0, 0.0), -turnAngle);
                break;
            case 92:    // \ roll left
                currentState.transform = currentState.transform * rotationMatrix(vec3(1.0, 0.0, 0.0), turnAngle);
                break;
            case 47:    // / roll right
                currentState.transform = currentState.transform * rotationMatrix(vec3(1.0, 0.0, 0.0), -turnAngle);
                break;
            case 91:    // [ push state
                stack[++top] = currentState;              
                break;
            case 93:    // ] pop state
                currentState = stack[top--];
                idx = currentState.treeIdx;
                break;
            case 33:    // ! decrement segment width
                currentState.width *= 0.8;
                break;
            case 76:    // L (BIG L) make leaf

                break;
            default:    // go forward, add new node to tree, connect to parent
                //check child list for available entry
                //create child node, set parent and other data
                //increment lastIdx
                lastIdx++;
                vec3 dir = currentState.transform[0].xyz;
                currentState.transform = translate(0.2 * dir) * currentState.transform;
                tree[lastIdx].pos = currentState.transform[3];
                tree[lastIdx].parent = idx;
                tree[lastIdx].width = currentState.width;
                for (int i = 0; i < 4; i++) {
                    if (tree[idx].children[i] != 0) {
                        tree[idx].children[i] = lastIdx;
                        break;
                    }
                }
                idx = lastIdx;
                //update bounding box bounds
                for (int j = 0; j < 3; j++) {
                    if (tree[idx].pos[j] < boxMin[j]) boxMin[j] = tree[idx].pos[j];
                    if (tree[idx].pos[j] > boxMax[j]) boxMax[j] = tree[idx].pos[j];
                }
                break;
        };
    }
    //write bounding box buffer data
    uint boxIdx = 0;
    vec4 c00 = vec4(boxMin.x, boxMin.y, boxMin.z, 1.0);
    vec4 c01 = vec4(boxMin.x, boxMax.y, boxMin.z, 1.0);
    vec4 c10 = vec4(boxMax.x, boxMin.y, boxMin.z, 1.0);
    vec4 c11 = vec4(boxMax.x, boxMax.y, boxMin.z, 1.0); 
    boxVertices[boxIdx++] = c00;
    boxVertices[boxIdx++] = c01;
    boxVertices[boxIdx++] = c10;
    boxVertices[boxIdx++] = c10;
    boxVertices[boxIdx++] = c01;
    boxVertices[boxIdx++] = c11;

    c00 = vec4(boxMin.x, boxMin.y, boxMax.z, 1.0);
    c01 = vec4(boxMin.x, boxMax.y, boxMax.z, 1.0);
    c10 = vec4(boxMax.x, boxMin.y, boxMax.z, 1.0);
    c11 = vec4(boxMax.x, boxMax.y, boxMax.z, 1.0); 
    boxVertices[boxIdx++] = c00;
    boxVertices[boxIdx++] = c01;
    boxVertices[boxIdx++] = c10;
    boxVertices[boxIdx++] = c10;
    boxVertices[boxIdx++] = c01;
    boxVertices[boxIdx++] = c11;

    c00 = vec4(boxMin.x, boxMin.y, boxMin.z 1.0);
    c01 = vec4(boxMin.x, boxMin.y, boxMax.z 1.0);
    c10 = vec4(boxMin.x, boxMax.y, boxMin.z 1.0);
    c11 = vec4(boxMin.x, boxMax.y, boxMax.z 1.0); 
    boxVertices[boxIdx++] = c00;
    boxVertices[boxIdx++] = c01;
    boxVertices[boxIdx++] = c10;
    boxVertices[boxIdx++] = c10;
    boxVertices[boxIdx++] = c01;
    boxVertices[boxIdx++] = c11;

    c00 = vec4(boxMax.x, boxMin.y, boxMin.z, 1.0);
    c01 = vec4(boxMax.x, boxMin.y, boxMax.z, 1.0);
    c10 = vec4(boxMax.x, boxMax.y, boxMin.z, 1.0);
    c11 = vec4(boxMax.x, boxMax.y, boxMax.z, 1.0); 
    boxVertices[boxIdx++] = c00;
    boxVertices[boxIdx++] = c01;
    boxVertices[boxIdx++] = c10;
    boxVertices[boxIdx++] = c10;
    boxVertices[boxIdx++] = c01;
    boxVertices[boxIdx++] = c11;

    c00 = vec4(boxMin.x, boxMin.y, boxMin.z, 1.0);
    c01 = vec4(boxMin.x, boxMin.y, boxMax.z, 1.0);
    c10 = vec4(boxMax.x, boxMin.y, boxMin.z, 1.0);
    c11 = vec4(boxMax.x, boxMin.y, boxMax.z, 1.0); 
    boxVertices[boxIdx++] = c00;
    boxVertices[boxIdx++] = c01;
    boxVertices[boxIdx++] = c10;
    boxVertices[boxIdx++] = c10;
    boxVertices[boxIdx++] = c01;
    boxVertices[boxIdx++] = c11;

    c00 = vec4(boxMin.x, boxMax.y, boxMin.z, 1.0);
    c01 = vec4(boxMin.x, boxMax.y, boxMax.z, 1.0);
    c10 = vec4(boxMax.x, boxMax.y, boxMin.z, 1.0);
    c11 = vec4(boxMax.x, boxMax.y, boxMax.z, 1.0); 
    boxVertices[boxIdx++] = c00;
    boxVertices[boxIdx++] = c01;
    boxVertices[boxIdx++] = c10;
    boxVertices[boxIdx++] = c10;
    boxVertices[boxIdx++] = c01;
    boxVertices[boxIdx++] = c11;
}