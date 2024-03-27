#version 430

layout(location = 0) in vec4 pos;

uniform layout(location = 0) mat4 Model;
uniform layout(location = 1) mat4 MVP;

out vec3 worldPos;
flat out uint treeIdx;

void main() {
    vec4 vertPos = vec4(pos.xyz, 1.0);
    worldPos = (Model * vertPos).xyz;
    treeIdx = floatBitsToUint(pos[3]);
    gl_Position = MVP * vertPos;
}