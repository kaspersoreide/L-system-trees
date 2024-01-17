#include "tree.h"
#include <iostream>

Tree::Tree() {
    Lsystem lsystem;
	lsystem.addRule('F', "F[+F]/[+F]/[^F]F", 1.0f);
	lsystem.setAxiom("F");
	lsystem.iterate(5);
    GLuint stringBuffer;
    glGenBuffers(1, &stringBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, stringBuffer);
    vector<uint32_t> uintString;
    for (char c : lsystem.product) uintString.push_back(c);
    int stringSize = uintString.size();
    glBufferData(GL_SHADER_STORAGE_BUFFER, stringSize * sizeof(uint32_t), uintString.data(), GL_STATIC_DRAW);
	float angle = 20;
	Turtle turtle(0.55f, 0.2f, PI * angle / 180);
	turtle.buildGPU(stringBuffer);
    VAO = turtle.VAO;
    vertexCount = turtle.vertexCount;
}