#version 430
#define PI2 6.28318530717958647693
/*
    this shader takes in tree buffer and writes vertices for every branch.
    each branch is rendered as a cubic bezier cylinder segment.

*/

layout (local_size_x = 1, local_size_y = 1) in;

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
    vec4 vertices[];   
};

layout (binding = 2) coherent writeonly buffer block2
{
    uint indices[];
};

layout (binding = 3) coherent writeonly buffer block3
{
    //last component is 0, so this vector is invariant under translation
    vec4 normals[];   
};


uniform layout(location = 0) uint segmentsPerNode;
uniform layout(location = 1) uint verticesPerSegment;

vec3 cubicSpline(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t) {
    //B(t) = (1 - t)^3 P0 + 3 (1 - t)^2 t P1 + 3 (1 - t) t^2 P2 + t^3 P3
    float t_inv = 1.0 - t;
    return t_inv*t_inv*t_inv * p0
        + 3*t_inv*t_inv*t * p1
        + 3*t_inv*t*t * p2
        + t*t*t * p3;
}

vec3 splineDerivative(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t) {
    float t_inv = 1.0 - t;
    return 3 * t_inv * t_inv * (p1 - p0)
        + 6 * t_inv * t * (p2 - p1)
        + 3 * t * t * (p3 - p2);
}

vec3 hermiteSpline(vec3 p0, vec3 p1, vec3 m0, vec3 m1, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    return (2. * t3 - 3. * t2 + 1.) * p0
         + (t3 - 2. * t2 + t) * m0
         + (-2. * t3 + 3. * t2) * p1
         + (t3 - t2) * m1;
}

vec3 hermiteDerivative(vec3 p0, vec3 p1, vec3 m0, vec3 m1, float t) {
    float t2 = t * t;
    return (6 * t2 - 6 * t) * p0
         + (3 * t2 - 4 * t + 1) * m0
         + (-6 * t2 + 6 * t) * p1
         + (3 * t2 - 2 * t) * m1;
}

mat3 rotationMatrix(vec3 axis, float angle) {
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

    return mat3(
        vec3(ocxx + c, ocxy - sz, ocxz + sy),
        vec3(ocxy + sz, ocyy + c, ocyz - sx),
        vec3(ocxz - sy, ocyz + sx, oczz + c)
    );
}

void main() {
    uint idx = gl_GlobalInvocationID.x + 1;
    float t = float(gl_GlobalInvocationID.y) / (segmentsPerNode);
    vec3 p1 = tree[idx].T[3].xyz;
    vec3 p0 = tree[tree[idx].parent].T[3].xyz;
    float width1 = tree[idx].width;
    float width0 = tree[tree[idx].parent].width; 
    vec3 m0 = tree[tree[idx].parent].T[0].xyz;
    vec3 m1 = tree[idx].T[0].xyz;
    float currentWidth = mix(width0, width1, t);

    vec3 center = hermiteSpline(p0, p1, m0, m1, t);//cubicSpline(p0, p1, p2, p3, t);
    //f is forward, along tangent
    vec3 f = hermiteDerivative(p0, p1, m0, m1, t);//normalize(splineDerivative(p0, p1, p2, p3, t));
    vec3 r = normalize(cross(f, vec3(-f.y, f.z, f.x)));
    mat3 R = rotationMatrix(f, PI2 / verticesPerSegment);
    //TODO double check vIdx
    uint vIdx = (gl_GlobalInvocationID.x * segmentsPerNode + gl_GlobalInvocationID.y) * verticesPerSegment;
    uint pIdx = (tree[idx].parent) * segmentsPerNode * verticesPerSegment;
    //if (gl_GlobalInvocationID.y == 0) return;
    for (int i = 0; i < verticesPerSegment; i++) {
        vertices[vIdx + i] = vec4(center + currentWidth * r, uintBitsToFloat(idx));
        normals[vIdx + i] = vec4(r, 0.0);
        r = R * r;
        uint iIdx = (vIdx + i) * 6;
        uint offset = (i < verticesPerSegment - 1) ? i + 1 : 0;
        //if (gl_GlobalInvocationID.y == 0) continue;
        indices[iIdx++] = vIdx + i;
        indices[iIdx++] = (gl_GlobalInvocationID.y == 0) 
            ? pIdx - verticesPerSegment + i 
            : vIdx - verticesPerSegment + i;
        indices[iIdx++] = vIdx + offset;

        indices[iIdx++] = (gl_GlobalInvocationID.y == 0) 
            ? pIdx - verticesPerSegment + i
            : vIdx - verticesPerSegment + i;
        indices[iIdx++] = (gl_GlobalInvocationID.y == 0) 
            ? pIdx - verticesPerSegment + offset
            : vIdx - verticesPerSegment + offset;
        indices[iIdx++] = vIdx + offset;
    }
    
}