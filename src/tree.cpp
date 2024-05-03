#include "tree.h"
#include <iostream>
#include "loadshaders.h"

void Tree::generateLeafVAO() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, turtle->leafModelsBuffer);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t), &lastLeafIdx);
    
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //generate leaf geometry and put into vbo
    vector<vec4> positions;
    float leafSize = 0.2f;
    vec4 middle = vec4(leafSize / 2, 0.0, 0.0, 1.0);
    positions.push_back(vec4(0.0, 0.0, 0.0, 1.0));
    positions.push_back(vec4(leafSize / 2, 0.0, leafSize / 2, 1.0));
    positions.push_back(middle);
    positions.push_back(vec4(leafSize / 2, 0.0, leafSize / 2, 1.0));
    positions.push_back(vec4(leafSize, 0.0, 0, 1.0));
    positions.push_back(middle);
    positions.push_back(vec4(leafSize, 0.0, 0, 1.0));
    positions.push_back(middle);
    positions.push_back(vec4(leafSize / 2, 0.0, -leafSize / 2, 1.0));
    positions.push_back(vec4(leafSize / 2, 0.0, -leafSize / 2, 1.0));
    positions.push_back(middle);
    positions.push_back(vec4(0.0, 0.0, 0.0, 1.0));
    leafVertexCount = positions.size();
    glBufferData(GL_ARRAY_BUFFER, positions.size() * 4 * sizeof(float), positions.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &leafVAO);
    glBindVertexArray(leafVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
}


Tree::Tree(float branchAngle, float initialWidth, float widthDecay, int iterations, int type) {
	//
    lsystem = new Lsystem();
    lsystem->setAxiom("A");
    lsystem->addRule('A', "[&F[###^^L]!A]/////#[&F[###^^L]!A]///////#[&F[###^^L]!A]", 1.0f);
    lsystem->addRule('F', "S/////F", 1.0f);
    lsystem->addRule('S', "F[###^^L]", 1.0f);
    //lsystem.addRule('S', "FA", 0.5f);
    //lsystem->addRule('F', "F[+!/FL]/^F[-!/FL]/^F", 1.0f);
	//lsystem->setAxiom("F");

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

    generateLeafVAO();
}

void Tree::render(GLuint shader, mat4 Model, mat4 VP, vec3 camPos, GLuint leafShader) {
    mat4 MVP = VP * Model;

    //leaves
    glBindVertexArray(leafVAO);
    glUseProgram(leafShader);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, turtle->leafModelsBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, turtle->leafModelsBuffer);
    glUniformMatrix4fv(0, 1, GL_FALSE, &MVP[0][0]);
    glDrawArraysInstanced(GL_TRIANGLES, 0, leafVertexCount, lastLeafIdx + 1);
    //glDrawArrays(GL_TRIANGLES, 0, leafVertexCount);

    //tree
    glUseProgram(shader);
	glUniformMatrix4fv(0, 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(1, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(3, 1, GL_FALSE, &VP[0][0]);
	glUniform3fv(2, 1, &camPos[0]);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, turtle->treeBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, turtle->treeBuffer);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36 * lastIdx);
}