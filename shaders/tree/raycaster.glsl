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

float cylinderDist(uint idx, vec3 point) {
    //approximate by cylinder using distance from point to line
    //https://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
    vec3 x2 = (Model * tree[idx].pos).xyz;
    vec3 x1 = (Model * tree[tree[idx].parent].pos).xyz;
    vec3 cp = cross(point - x1, point - x2);
    float d = length(cross(point - x1, point - x2)) / distance(x2, x1); 
    return d - tree[idx].width;
}

vec3 cylinderNormal(uint idx, vec3 point) {
    //https://gdbooks.gitbooks.io/3dcollisions/content/Chapter1/closest_point_on_line.html
    vec3 a = (Model * tree[idx].pos).xyz;
    vec3 b = (Model * tree[tree[idx].parent].pos).xyz;
    vec3 ab = b - a;
    float t = dot(point - a, ab) / dot(ab, ab);
    vec3 closestPoint = a + ab * t;
    return normalize(point - closestPoint);
}

void main() {
    //vec3 closestPoint = (Model * tree[treeIdx].pos).xyz;
    vec3 viewDir = normalize(worldPos - cameraPos);
    vec3 rayPos = worldPos;
    int steps = 0;
    float dist = 10000000.0;//distance(rayPos, closestPoint);
    float epsilon = 0.001;
    while (dist > epsilon && steps < 10) {
        //vec3 a = (Model * tree[treeIdx].pos).xyz;

        dist = cylinderDist(treeIdx, rayPos);
        rayPos += dist * viewDir;
        steps++;
    }
    if (dist > epsilon) {
        discard;
        return;
    }
    vec3 color = vec3(0.5, 0.25, 0.0);
    vec3 lightDir = vec3(1.0, 0.0, 0.0);
    float brightness = dot(lightDir, cylinderNormal(treeIdx, rayPos));
    FragColor = vec4(
        brightness * exp(-dist) * color,
        //color,
        1.0
    );
}