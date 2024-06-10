#pragma once
#include "glutils.h"
#include "lsystem.h"
#include <stack>

extern GLuint treeBuildingShader;

class Turtle {
public:
    struct State {
        vec2 pos;
        float angle;
        float width;
    };

    Turtle(float initialWidth, float widthDecay, float rotationAngle);
    ~Turtle();
    void build(string buildString);
    void buildGPU(GLuint stringBuffer, float segmentLength);
    void pushState();
    void popState();

    vector<vec2> vertices; //line points for GL_LINES
    GLuint treeBuffer, boxVAO, leafModelsBuffer;
    

    State state;
    stack<State> stateStack;
    float rotationAngle;
    float widthDecay;
};