#version 430 core

layout (local_size_x = 1) in;

layout (binding = 0) coherent readonly buffer block1
{
    uint string[];
};

layout (binding = 1) coherent writeonly buffer block2
{
    vec4 vertices[];    //line segments
};

uniform layout (location = 0) uint stringLength;
uniform layout (location = 1) float segmentLength;
uniform layout (location = 2) float turnAngle;

mat4 rotate(float a) {
    return mat4(
        cos(a), -sin(a), 0.0, 0.0,
        sin(a), cos(a), 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
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




void main() {
    int top = -1;
    vec4 stateStack[128]; //x and y is pos, z is angle, w is step
    vec4 state;   
    state.w = segmentLength;
    //float angle = 0.1;
    for (uint i = 0; i < stringLength; i++) {
        switch (string[i]) {
            case 43:    //+ 
                state.z += turnAngle;
                break;
            case 45:    //-
                state.z -= turnAngle;
                break;
            case 91:    //[
                stateStack[++top] = state;
                state.w *= 0.8;
                break;
            case 93:    //]
                state = stateStack[top--];
                break; 
            default:    //go forward
                vertices[2 * i] = vec4(state.xy, 0.0, 1.0);
                vec2 forward = vec2(cos(state.z), sin(state.z));
                state.xy += state.w * forward;
                vertices[2 * i + 1] = vec4(state.xy, 0.0, 1.0);
                break;
        };
        //vertices[2 * i] = vec4(0.001 * i, 0.0, 0.0, 1.0);
        //vertices[2 * i + 1] = vec4(0.001 * i + 0.0005, 0.0, 0.0, 1.0);
    }
}