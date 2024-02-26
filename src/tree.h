#pragma once
#include "glutils.h"
#include "turtle.h"
#include "lsystem.h"

class Tree {
public:
    Tree();
    Tree(float branchAngle, float initialWidth, float widthDecay, int iterations = 5, int type = 0);
    GLuint VAO, vertexCount;
};