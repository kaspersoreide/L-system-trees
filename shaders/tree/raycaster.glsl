#version 430

in vec3 worldPos;
flat in uint treeIdx;
out vec4 FragColor;

uniform layout(location = 0) mat4 Model;
uniform layout(location = 2) vec3 cameraPos;

struct Node {
    uint idx;
    uint parent;
    uvec4 children;     //max 4 children
    vec4 pos;
    float width;
};

layout (binding = 0) coherent readonly buffer block0
{
    uint lastIdx;   //index of last node in buffer
    Node tree[];   
};

float branchDist(uint idx, vec3 point) {
    return distance(point, (Model * tree[idx].pos).xyz) - tree[idx].width;
}

/*
uint getNearestChild(uint treeIdx, vec3 point) {
    float minDist = branchDist(treeIdx, point);
    uint nearest = treeIdx;
    for (uint i = 0; i < 4; i++) {
        uint childIdx = tree[treeIdx].children[i];
        if (childIdx == 0) break;
        float dist = branchDist(childIdx, point);
        if (dist < minDist) {
            minDist = dist;
            nearest = childIdx;
        }
    }
    return nearest;
}
*/

void main() {
    vec3 closestPoint = (Model * tree[treeIdx].pos).xyz;
    vec3 viewDir = normalize(worldPos - cameraPos);
    vec3 rayPos = worldPos;
    int steps = 0;
    float dist = 10000000.0;//distance(rayPos, closestPoint);
    while (dist > 0.1 && steps < 20) {
        
        dist = branchDist(treeIdx, rayPos);
        
        rayPos += dist * viewDir;
        steps++;
    }
    vec3 color = vec3(0.5, 0.25, 0.0);
    FragColor = vec4(
        0.1 + exp(-dist) * color,
        1.0
    );
}