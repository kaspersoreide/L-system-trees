#version 430 core

layout (local_size_x = 1) in;

layout (binding = 0) coherent readonly buffer block0
{
    uint string[];
};

layout (binding = 1) coherent writeonly buffer block1
{
    vec4 vertices[];
};

layout (binding = 2) coherent writeonly buffer block2
{
    vec4 normals[];
};

layout (binding = 3) coherent writeonly buffer block3
{
    vec4 colors[];
};

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
    mat4 T;
    float width;
    float colorGradient;
};

void main() {
    int top = -1;
    State stateStack[32];
    State state;
    //set initial state pointing upwards (y-axis), because the y axis is always up and trees grow upwards
    state.T = mat4(
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
    state.width = branchWidth;
    state.colorGradient = 0.0;
    vec4 brown = vec4(0.588, 0.294, 0.1, 1.0);
    vec4 green = vec4(0.0, 1.0, 0.0, 1.0);
    
    for (uint i = 0; i < stringLength; i++) {
        switch (string[i]) {
            case 43:    // + turn left
                state.T = state.T * rotationMatrix(vec3(0.0, 0.0, 1.0), turnAngle);
                break;
            case 45:    // - turn right
                state.T = state.T * rotationMatrix(vec3(0.0, 0.0, 1.0), -turnAngle);
                break;
            case 38:    // & pitch down
                state.T = state.T * rotationMatrix(vec3(0.0, 1.0, 0.0), turnAngle);
                break;
            case 94:    // ^ pitch up
                state.T = state.T * rotationMatrix(vec3(0.0, 1.0, 0.0), -turnAngle);
                break;
            case 92:    // \ roll left
                state.T = state.T * rotationMatrix(vec3(1.0, 0.0, 0.0), turnAngle);
                break;
            case 47:    // / roll right
                state.T = state.T * rotationMatrix(vec3(1.0, 0.0, 0.0), -turnAngle);
                break;
            case 91:    // [ push state
                stateStack[++top] = state;                
                break;
            case 93:    // ] pop state
                state = stateStack[top--];
                break;
            case 33:    // ! decrement segment width
                state.width *= 0.8;
                state.colorGradient += 0.04;
                break;
            case 76:    // L (BIG L) make leaf
                float leafLength = 0.1;//1 * state.width;
                vec3 fwd = state.T[0].xyz;
                vec3 up = state.T[1].xyz;
                vec3 right = state.T[2].xyz;
                vec3 p[7];
                p[0] = state.T[3].xyz; 
                p[1] = p[0] + leafLength * (fwd + right);
                p[2] = p[1] + leafLength * (fwd);
                p[3] = p[2] + leafLength * (fwd - right);
                p[4] = p[3] + leafLength * (-fwd - right);
                p[5] = p[4] + leafLength * (-fwd);
                p[6] = p[0];
                vec3 middle = 0.5 * (p[0] + p[3]);
                
                for (int j = 0; j < 6; j++) {
                    uint idx = cylinderSegments * 6 * i + 3 * j; 
                    vertices[idx] = vec4(middle, 1.0);
                    normals[idx] = vec4(up, 0.0);
                    colors[idx] = green;
                    vertices[idx + 1] = vec4(p[j + 1], 1.0);
                    normals[idx + 1] = vec4(up, 0.0);
                    colors[idx + 1] = green;
                    vertices[idx + 2] = vec4(p[j], 1.0);
                    normals[idx + 2] = vec4(up, 0.0);
                    colors[idx + 2] = green;
                }
                break;
            default:    // go forward
                state.colorGradient += 0.001;
                vec3 center0 = (state.T[3]).xyz; 
                vec3 forward = (state.T[0]).xyz;
                state.T = translate(0.2 * forward) * state.T;
                vec3 center1 = (state.T[3]).xyz;
                float radius0 = state.width;
                state.width *= 0.95;
                float radius1 = state.width;
                for (int j = 0; j < cylinderSegments; j++) {
                    uint idx = cylinderSegments * 6 * i + 6 * j;
                    float angle0 = j * 2 * PI / cylinderSegments;
                    float angle1 = (j + 1) * 2 * PI / cylinderSegments;
                    vec3 r0 = (state.T * vec4(0.0, cos(angle0), sin(angle0), 0.0)).xyz;
                    vec3 r1 = (state.T * vec4(0.0, cos(angle1), sin(angle1), 0.0)).xyz;
                    vec4 p0 = vec4(center0 + radius0 * r0, 1.0);
                    vec4 p1 = vec4(center0 + radius0 * r1, 1.0);
                    vec4 p2 = vec4(center1 + radius1 * r0, 1.0);
                    vec4 p3 = vec4(center1 + radius1 * r1, 1.0);
                    vec4 normal0 = vec4(normalize(r0), 0.0);
                    vec4 normal1 = vec4(normalize(r1), 0.0);
                    vec4 color = mix(brown, green, state.colorGradient);
                    vertices[idx] = p0;
                    normals[idx] = normal0;
                    colors[idx] = color;
                    vertices[idx + 1] = p1;
                    normals[idx + 1] = normal1;
                    colors[idx + 1] = color;
                    vertices[idx + 2] = p2;
                    normals[idx + 2] = normal0;
                    colors[idx + 2] = color;
                    vertices[idx + 3] = p3;
                    normals[idx + 3] = normal1;
                    colors[idx + 3] = color;
                    vertices[idx + 4] = p1;
                    normals[idx + 4] = normal1;
                    colors[idx + 4] = color;
                    vertices[idx + 5] = p2;
                    normals[idx + 5] = normal0;
                    colors[idx + 5] = color;
                }
                
                break;
        };
        //vertices[2 * i] = vec4(0.001 * i, 0.0, 0.0, 1.0);
        //vertices[2 * i + 1] = vec4(0.001 * i + 0.0005, 0.0, 0.0, 1.0);
    }
}