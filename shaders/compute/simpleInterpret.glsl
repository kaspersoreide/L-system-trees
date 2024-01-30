#version 430 core

layout (local_size_x = 1) in;

layout (binding = 0) coherent readonly buffer block1
{
    uint string[];
};

layout (binding = 1) coherent writeonly buffer block2
{
    vec4 vertices[];
};

layout (binding = 2) coherent writeonly buffer block3
{
    vec4 normals[];
};

uniform layout (location = 0) uint stringLength;
uniform layout (location = 1) float segmentLength;
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

struct State {
    mat4 transform;
    float segmentLength;
};

const float PI = 3.14159265358979323846;

void main() {
    int top = -1;
    State stateStack[32];
    State currentState;
    //set initial state pointing upwards (y-axis), because the y axis is always up and trees grow upwards
    currentState.transform = mat4(
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
    currentState.segmentLength = segmentLength;
    
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
                stateStack[++top] = currentState;
                //currentState.segmentLength *= 0.7;
                break;
            case 93:    // ] pop state
                currentState = stateStack[top--];
                break;
            case 33:    // ! decrement segment length
                currentState.segmentLength *= 0.8;
                break;
            case 76:    // L (BIG L) make leaf
                float leafLength = 0.2 * currentState.segmentLength;
                vec3 forward = currentState.transform[0].xyz;
                vec3 up = currentState.transform[1].xyz;
                vec3 right = currentState.transform[2].xyz;
                vec3 p0 = currentState.transform[3].xyz; //current translation = last vector in transform matrix
                //vec3 p1 = 
            default:    // go forward
                vec3 center0 = (currentState.transform * vec4(0.0, 0.0, 0.0, 1.0)).xyz; 
                vec3 forward = (currentState.transform * vec4(1.0, 0.0, 0.0, 0.0)).xyz;
                currentState.transform = translate(0.2 * forward) * currentState.transform;
                vec3 center1 = (currentState.transform * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
                float radius0 = currentState.segmentLength / 10; 
                currentState.segmentLength *= 0.99;
                float radius1 = currentState.segmentLength / 10; 
                for (int j = 0; j < cylinderSegments; j++) {
                    uint idx = cylinderSegments * 6 * i + 6 * j;
                    float angle0 = j * 2 * PI / cylinderSegments;
                    float angle1 = (j + 1) * 2 * PI / cylinderSegments;
                    vec3 r0 = (currentState.transform * vec4(0.0, cos(angle0), sin(angle0), 0.0)).xyz;
                    vec3 r1 = (currentState.transform * vec4(0.0, cos(angle1), sin(angle1), 0.0)).xyz;
                    vec4 p0 = vec4(center0 + radius0 * r0, 1.0);
                    vec4 p1 = vec4(center0 + radius0 * r1, 1.0);
                    vec4 p2 = vec4(center1 + radius1 * r0, 1.0);
                    vec4 p3 = vec4(center1 + radius1 * r1, 1.0);
                    vertices[idx] = p0;
                    normals[idx] = vec4(r0, 0.0);
                    vertices[idx + 1] = p1;
                    normals[idx + 1] = vec4(r1, 0.0);
                    vertices[idx + 2] = p2;
                    normals[idx + 2] = vec4(r0, 0.0);
                    vertices[idx + 3] = p3;
                    normals[idx + 3] = vec4(r1, 0.0);
                    vertices[idx + 4] = p1;
                    normals[idx + 4] = vec4(r1, 0.0);
                    vertices[idx + 5] = p2;
                    normals[idx + 5] = vec4(r0, 0.0);
                }
                
                break;
        };
        //vertices[2 * i] = vec4(0.001 * i, 0.0, 0.0, 1.0);
        //vertices[2 * i + 1] = vec4(0.001 * i + 0.0005, 0.0, 0.0, 1.0);
    }
}