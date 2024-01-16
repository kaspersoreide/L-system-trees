#pragma once
#include "glutils.h"
#include "lsystem.h"
#include <stack>

class Turtle {
public:
    struct State {
        vec2 pos;
        float angle;
        float step;
    };

    Turtle(float initialStep, float stepDecay, float rotationAngle);
    void build(string buildString);
    void buildGPU(GLuint stringBuffer);
    void pushState();
    void popState();

    vector<vec2> vertices; //line points for GL_LINES
    GLuint VAO, vertexCount;

    State state;
    stack<State> stateStack;
    float rotationAngle;
    float stepDecay;
};