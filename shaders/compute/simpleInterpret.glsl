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
    mat4 T;
    float width;
};

layout (binding = 1) coherent buffer block1
{
    uint lastIdx;   //index of last node in buffer
    Node tree[];   
};

layout (binding = 2) coherent writeonly buffer block2 
{
    uint lastLeafIdx;
    mat4 Models[];
};

/*
layout (binding = 2) coherent writeonly buffer block2
{
    vec4 boxVertices[36];   //6 vertices per face, 6 faces
};
*/

uniform layout (location = 0) uint stringLength;
uniform layout (location = 1) float branchWidth;
uniform layout (location = 2) float turnAngle;
uniform layout (location = 3) int cylinderSegments;
uniform layout (location = 4) float branchLength;

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

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash(uint x) {
	x += (x << 10u);
	x ^= (x >> 6u);
	x += (x << 3u);
	x ^= (x >> 11u);
	x += (x << 15u);
	return x;
}

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct(uint m) {
	const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
	const uint ieeeOne = 0x3F800000u; // 1.0 in IEEE binary32

	m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
	m |= ieeeOne;                          // Add fractional part to 1.0

	float  f = uintBitsToFloat(m);       // Range [1:2]
	return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float random(uint x) { return floatConstruct(hash(x)); }

vec3 randomVec3(uint s) {
    float x = random(s);
    s = hash(s);
    float y = random(s);
    s = hash(s);
    float z = random(s);
    return normalize(vec3(x, y, z) - 0.5);
}

const float PI = 3.14159265358979323846;

struct State {
    mat4 transform;
    float width;
    uint treeIdx;
};

void main() {
    int top = -1;
    State stack[16];
    State currentState;
    //set initial state pointing upwards (y-axis), because the y axis is always up and trees grow upwards
    currentState.transform = mat4(
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
    currentState.treeIdx = 0;
    currentState.width = branchWidth;
    tree[0].T = currentState.transform;
    tree[0].width = currentState.width;
    tree[0].parent = 0;
    tree[0].children = uvec4(0);
    lastIdx = 0;
    uint leafIdx = 0;
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
                //put leaf at end of branch
                Models[leafIdx++] = currentState.transform;
                currentState = (top >= 0) ? stack[top--] : currentState;
                
                break;
            case 33:    // ! decrement segment width
                currentState.width *= 0.8;
                break;
            case 76:    // L (BIG L) make leaf
                Models[leafIdx++] = currentState.transform;
                break;
            case 70:    // F (BIG F) go forward, add new node to tree, connect to parent
                //increment lastIdx
                lastIdx++;
                //move currentstate forward
                vec3 dir = currentState.transform[0].xyz;
                //float pull = 1.0;//- abs(dot(dir, vec3(0.0, -1.0, 0.0)));
                //currentState.transform = translate(branchLength * dir + 0.1 * pull * currentState.width * vec3(0.0, -1.0, 0.0)) * currentState.transform;
                uint seed = leafIdx + lastIdx + top;
                vec3 fluctuation = 0.2 * randomVec3(seed); 
                vec3 axis = cross(dir, vec3(0.0, 1.0, 0.0));
                float angle = 2 * max(dot(dir, vec3(0.0, -1.0, 0.0)), 0.0);
                if (dot(axis, axis) > 0.001) {
                    currentState.transform = currentState.transform * rotationMatrix(axis, angle);
                }
                //vec3 tropism = 0.6 * vec3(0.0, 1.0, 0.0);
                currentState.transform = translate(branchLength * normalize(dir + fluctuation)) * currentState.transform;
                //create child node, set parent and other data
                tree[lastIdx].T = currentState.transform;
                tree[lastIdx].parent = currentState.treeIdx;
                tree[lastIdx].width = currentState.width;
                tree[lastIdx].children = uvec4(0);
                for (int i = 0; i < 4; i++) {
                    //check child list for available entry and insert child
                    if (tree[currentState.treeIdx].children[i] == 0) {
                        tree[currentState.treeIdx].children[i] = lastIdx;
                        break;
                    }
                }
                currentState.treeIdx = lastIdx;
                //turn downwards based on gravity
                break;
            default:
                break;
        };
    }
    lastLeafIdx = leafIdx;
}