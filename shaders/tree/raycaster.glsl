#version 430

in vec3 worldPos;
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

void main() {
    vec3 closestPoint = (Model * tree[0].pos).xyz;
    vec3 viewDir = normalize(worldPos - cameraPos);
    vec3 rayPos = cameraPos;//worldPos;
    int steps = 0;
    float dist = 10000000.0;//distance(rayPos, closestPoint);
    while (dist > 0.1 && steps < 20) {
        float minDist = 10000000000.0;
        for (uint i = 0; i < lastIdx; i++) {
            vec3 p = (Model * tree[i].pos).xyz;
            dist = distance(p, rayPos);
            if (dist < minDist) {
                minDist = dist;
                closestPoint = p;
            }
        }
        dist = distance(rayPos, closestPoint) - 0.3;
        rayPos += dist * viewDir;
        steps++;
    }
    FragColor = vec4(
        0.1 + vec3(exp(-dist)),
        1.0
    );
}