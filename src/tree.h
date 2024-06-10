#pragma once
#include "glutils.h"
#include "turtle.h"
#include "lsystem.h"

extern GLuint genLeafShader;
extern GLuint generateSplinesShader;

class Tree {
public:
    Tree(vec3 position, float scale, float branchAngle = 22.5, float initialWidth = 0.2, float widthDecay = 0.97, int iterations = 5, float segLength = 0.7);
    void render(GLuint shader, mat4 VP, vec3 camPos, GLuint leafShader);
protected:
    void generateLeafVertexArray();
    void generateLeafTexture();
    void generateSplines();
    void generateBoundingBoxes();
    Lsystem* lsystem;
    Turtle* turtle;
    mat4 Model;
    vec3 pos;
    GLuint vertexArray, leafVertexArray, leafTexture, elementBuffer;
    uint lastIdx, lastLeafIdx, leafVertexCount, indexCount;
    uint segmentsPerNode, verticesPerSegment;
    int seed;
};

