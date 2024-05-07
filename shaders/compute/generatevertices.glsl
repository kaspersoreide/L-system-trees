#version 430

/*
    this shader takes in tree buffer and writes vertices for every branch.
    each branch is rendered as a box (bounding box).
    bounding box goes from one node to its parent with padding on all sides equal to node width
    current implementation creates non-rectangular boxes if parent and child have different width
*/

layout (local_size_x = 1) in;

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

layout (binding = 1) coherent writeonly buffer block1
{
    //combined as vec3 for vertex position and last float as encoded index into tree[]
    //so last coordinate is uint bits as float of index of child node (box goes from child to parent)
    vec4 vertices[];   
};


void main() {
    uint idx = gl_GlobalInvocationID.x + 1;
    vec3 p1 = tree[idx].T[3].xyz;
    vec3 p0 = tree[tree[idx].parent].T[3].xyz;
    float width1 = 1.2 * tree[idx].width;
    float width0 = 1.2 * tree[tree[idx].parent].width; 
    //construct orthonormal frame u, v, w from transform matrix. 
    vec3 u1 = tree[idx].T[0].xyz;
    vec3 v1 = tree[idx].T[1].xyz;
    vec3 w1 = tree[idx].T[2].xyz;

    vec3 u0 = tree[tree[idx].parent].T[0].xyz;
    vec3 v0 = tree[tree[idx].parent].T[1].xyz;
    vec3 w0 = tree[tree[idx].parent].T[2].xyz;
    uint vIdx = 36 * gl_GlobalInvocationID.x; //36 vertices per segment


    
    //bottom corners
    vec3 c000 = p0 - width0 * v0 - width0 * w0;
    vec3 c001 = p0 - width0 * v0 + width0 * w0;
    vec3 c010 = p0 + width0 * v0 - width0 * w0;
    vec3 c011 = p0 + width0 * v0 + width0 * w0;
    //top corners
    vec3 c100 = p1 - width1 * v1 - width1 * w1;
    vec3 c101 = p1 - width1 * v1 + width1 * w1;
    vec3 c110 = p1 + width1 * v1 - width1 * w1;
    vec3 c111 = p1 + width1 * v1 + width1 * w1;
    
    //write vertices for all 6 sides of box
    //TODO: figure out how to parallelize further, make each thread write one vertex
    float fIdx = uintBitsToFloat(idx);
    vertices[vIdx++] = vec4(c100, fIdx);
    vertices[vIdx++] = vec4(c101, fIdx);
    vertices[vIdx++] = vec4(c110, fIdx);
    vertices[vIdx++] = vec4(c110, fIdx);
    vertices[vIdx++] = vec4(c101, fIdx);
    vertices[vIdx++] = vec4(c111, fIdx);

    vertices[vIdx++] = vec4(c000, fIdx);
    vertices[vIdx++] = vec4(c001, fIdx);
    vertices[vIdx++] = vec4(c010, fIdx);
    vertices[vIdx++] = vec4(c010, fIdx);
    vertices[vIdx++] = vec4(c001, fIdx);
    vertices[vIdx++] = vec4(c011, fIdx);

    vertices[vIdx++] = vec4(c000, fIdx);
    vertices[vIdx++] = vec4(c001, fIdx);
    vertices[vIdx++] = vec4(c100, fIdx);
    vertices[vIdx++] = vec4(c100, fIdx);
    vertices[vIdx++] = vec4(c001, fIdx);
    vertices[vIdx++] = vec4(c101, fIdx);

    vertices[vIdx++] = vec4(c010, fIdx);
    vertices[vIdx++] = vec4(c011, fIdx);
    vertices[vIdx++] = vec4(c110, fIdx);
    vertices[vIdx++] = vec4(c110, fIdx);
    vertices[vIdx++] = vec4(c011, fIdx);
    vertices[vIdx++] = vec4(c111, fIdx);

    vertices[vIdx++] = vec4(c000, fIdx);
    vertices[vIdx++] = vec4(c010, fIdx);
    vertices[vIdx++] = vec4(c100, fIdx);
    vertices[vIdx++] = vec4(c100, fIdx);
    vertices[vIdx++] = vec4(c010, fIdx);
    vertices[vIdx++] = vec4(c110, fIdx);

    vertices[vIdx++] = vec4(c001, fIdx);
    vertices[vIdx++] = vec4(c011, fIdx);
    vertices[vIdx++] = vec4(c101, fIdx);
    vertices[vIdx++] = vec4(c101, fIdx);
    vertices[vIdx++] = vec4(c011, fIdx);
    vertices[vIdx++] = vec4(c111, fIdx);

}