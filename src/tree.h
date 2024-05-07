#pragma once
#include "glutils.h"
#include "turtle.h"
#include "lsystem.h"

class Tree {
public:
    Tree(vec3 position, float scale, float branchAngle = 22.5, float initialWidth = 0.2, float widthDecay = 0.97, int iterations = 5, int type = 0);
    void render(GLuint shader, mat4 VP, vec3 camPos, GLuint leafShader);
protected:
    void generateLeafVAO();
    Lsystem* lsystem;
    Turtle* turtle;
    mat4 Model;
    GLuint VAO, leafVAO;
    uint32_t lastIdx, lastLeafIdx, leafVertexCount;
};