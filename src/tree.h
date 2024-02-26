#pragma once
#include "glutils.h"
#include "turtle.h"
#include "lsystem.h"

class Tree {
public:
    Tree(float branchAngle = 22.5, float initialWidth = 0.2, float widthDecay = 0.97, int iterations = 5, int type = 0);
    GLuint VAO, vertexCount;
};