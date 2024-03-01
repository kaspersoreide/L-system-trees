#include "tree.h"
#include <iostream>

Tree::Tree(float branchAngle, float initialWidth, float widthDecay, int iterations, int type) {
    Lsystem lsystem;
	//lsystem.addRule('F', "F[++!F]//[++!F]//[^^!F]F", 1.0f);
	//lsystem.setAxiom("F");
    lsystem.setAxiom("A");
    lsystem.addRule('A', "[&F[###^^L]!A]/////#[&F[###^^L]!A]///////#[&F[###^^L]!A]", 1.0f);
    lsystem.addRule('F', "S/////F", 1.0f);
    lsystem.addRule('S', "F[###^^L]", 1.0f);
    //lsystem.addRule('S', "FA", 0.5f);

    lsystem.iterate(iterations);
    GLuint stringBuffer;
    glGenBuffers(1, &stringBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, stringBuffer);
    vector<uint32_t> uintString;
    int nSegments = 0;
    int nLeaves = 0;
    for (char c : lsystem.product) {
        if (c == 'L') {
            nLeaves += 3;
        }
        if (c == 'F') {
            nSegments += 3;
        }
        uintString.push_back(c);
    }
    int stringSize = uintString.size();
    glBufferData(GL_SHADER_STORAGE_BUFFER, stringSize * sizeof(uint32_t), uintString.data(), GL_STATIC_DRAW);
	Turtle turtle(initialWidth, widthDecay, PI * branchAngle / 180);
	turtle.buildGPU(stringBuffer, 6, nSegments, nLeaves);
    VAO = turtle.VAO;
    vertexCount = turtle.vertexCount;
}