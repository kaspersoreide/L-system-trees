#include "tree.h"
#include <iostream>

Tree::Tree() {
    Tree(22.5f, 0.2, 0.97);
}

Tree::Tree(float branchAngle, float initialWidth, float widthDecay, int iterations, int type) {
    Lsystem lsystem;
	//lsystem.addRule('F', "F[++!F]//[++!F]//[^^!F]F", 1.0f);
	//lsystem.setAxiom("F");
    lsystem.setAxiom("A");
    lsystem.addRule('A', "[&FL!A]/////#[&FL!A]///////#[&FL!A]", 1.0f);
    lsystem.addRule('F', "S/////F", 1.0f);
    lsystem.addRule('S', "F[^^L]", 1.0f);

    lsystem.iterate(iterations);
    GLuint stringBuffer;
    glGenBuffers(1, &stringBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, stringBuffer);
    vector<uint32_t> uintString;
    for (char c : lsystem.product) uintString.push_back(c);
    int stringSize = uintString.size();
    glBufferData(GL_SHADER_STORAGE_BUFFER, stringSize * sizeof(uint32_t), uintString.data(), GL_STATIC_DRAW);
	Turtle turtle(initialWidth, widthDecay, PI * branchAngle / 180);
	turtle.buildGPU(stringBuffer);
    VAO = turtle.VAO;
    vertexCount = turtle.vertexCount;
}