#include "tree.h"
#include <iostream>
#include "loadshaders.h"

Tree::Tree(float branchAngle, float initialWidth, float widthDecay, int iterations, int type) {
	//
    lsystem = new Lsystem();
    //lsystem->setAxiom("A");
    //lsystem->addRule('A', "[&F[###^^L]!A]/////#[&F[###^^L]!A]///////#[&F[###^^L]!A]", 1.0f);
    //lsystem->addRule('F', "S/////F", 1.0f);
    //lsystem->addRule('S', "F[###^^L]", 1.0f);
    //lsystem.addRule('S', "FA", 0.5f);
    lsystem->addRule('F', "F[+!/F]F[-!/F]F", 1.0f);
	lsystem->setAxiom("F");

    lsystem->iterate(iterations);
    GLuint stringBuffer;
    glGenBuffers(1, &stringBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, stringBuffer);
    vector<uint32_t> uintString;
    for (char c : lsystem->product) {
        uintString.push_back(c);
    }
    int stringSize = uintString.size();
    glBufferData(GL_SHADER_STORAGE_BUFFER, stringSize * sizeof(uint32_t), uintString.data(), GL_STATIC_DRAW);
	turtle = new Turtle(initialWidth, widthDecay, PI * branchAngle / 180);
	turtle->buildGPU(stringBuffer, 6);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, turtle->treeBuffer);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t), &lastIdx);
    cout << "lastIdx: " << lastIdx << "\n";

    GLuint generateVerticesShader = loadComputeShader("shaders/compute/generatevertices.glsl");
    glUseProgram(generateVerticesShader);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, turtle->treeBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, turtle->treeBuffer);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, VBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, lastIdx * 36 * 4 * sizeof(float), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, VBO);

    glDispatchCompute(lastIdx, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

void Tree::render(GLuint shader, mat4 Model, mat4 MVP, vec3 camPos) {
    glUseProgram(shader);
	glUniformMatrix4fv(0, 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(1, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(2, 1, &camPos[0]);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, turtle->treeBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, turtle->treeBuffer);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36 * lastIdx);
}