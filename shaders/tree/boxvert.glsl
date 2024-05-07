#version 430

layout(location = 0) in vec4 pos;

uniform layout(location = 0) mat4 Model;
uniform layout(location = 1) mat4 MVP;
uniform layout(location = 3) mat4 VP;

out vec3 worldPos;
flat out uint treeIdx;
out float t_guess;
flat out vec3 nodeWorldPos;
flat out vec3 parentWorldPos;
flat out vec3 dir;
flat out vec3 parentDir;
flat out float width0;
flat out float width1;


struct Node {
    uint idx;
    uint parent;
    uvec4 children;     //max 4 children
    mat4 T;
    float width;
};

layout (binding = 0) coherent readonly buffer block0
{
    uint lastIdx;   //index of last node in buffer
    Node tree[];   
};

float closestParameter(vec3 a, vec3 b, vec3 point) {
    //a and b are endpoints of spline segment, p is point that we want to find closest t value for
    //return t value that is the parameter between 0 and 1, where 0 is a and 1 is b
    //approximated by a line
    vec3 ab = b - a;
    return dot(point - a, ab) / dot(ab, ab);
}

vec3 getParentDir(uint idx) {
    uint pIdx = tree[idx].parent;
    if (pIdx != 0) {
        vec3 p1 = tree[pIdx].T[3].xyz;
        vec3 p0 = tree[tree[pIdx].parent].T[3].xyz;
        return normalize(p1 - p0);
    }
    //if parent == 0 then parent is first node which grows upwards (positive y-axis)
    return vec3(0.0, 1.0, 0.0);
}

void main() {
    vec4 vertPos = vec4(pos.xyz, 1.0);
    worldPos = (Model * vertPos).xyz;
    uint idx = floatBitsToUint(pos[3]);
    vec3 p1 = (Model * tree[idx].T[3]).xyz;
    vec3 p0 = (Model * tree[tree[idx].parent].T[3]).xyz;
    dir = normalize(p1 - p0);
    parentDir = getParentDir(idx);
    t_guess = closestParameter(p0, p1, worldPos);
    nodeWorldPos = p1;
    parentWorldPos = p0;
    treeIdx = idx;
    width0 = tree[tree[idx].parent].width;
    width1 = tree[idx].width;
    
    gl_Position = MVP * vertPos;
}