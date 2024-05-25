#include "tree.h"
#include <iostream>
#include "loadshaders.h"
#include <glm/gtx/transform.hpp>

void Tree::generateLeafVertexArray() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, turtle->leafModelsBuffer);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t), &lastLeafIdx);
    
    
    //generate leaf geometry and put into vbo
    vector<vec4> positions;
    vector<vec2> uvs;
    positions.push_back(vec4(0.0, 0.0, -0.5, 1.0));
    uvs.push_back(vec2(0.0, 0.0));
    positions.push_back(vec4(1.0, 0.0, 0.5, 1.0));
    uvs.push_back(vec2(1.0, 1.0));
    positions.push_back(vec4(1.0, 0.0, -0.5, 1.0));
    uvs.push_back(vec2(0.0, 1.0));

    positions.push_back(vec4(0.0, 0.0, 0.5, 1.0));
    uvs.push_back(vec2(1.0, 0.0));
    positions.push_back(vec4(1.0, 0.0, 0.5, 1.0));
    uvs.push_back(vec2(1.0, 1.0));
    positions.push_back(vec4(0.0, 0.0, -0.5, 1.0));
    uvs.push_back(vec2(0.0, 0.0));
    leafVertexCount = positions.size();
    GLuint vertexBuffer, uvBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * 4 * sizeof(float), positions.data(), GL_STATIC_DRAW);
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * 2 * sizeof(float), uvs.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &leafVertexArray);
    glBindVertexArray(leafVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

void Tree::generateSplines() {
    GLuint generateSplinesShader = loadComputeShader("shaders/compute/generatesplines.glsl");
    glUseProgram(generateSplinesShader);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, turtle->treeBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, turtle->treeBuffer);

    segmentsPerNode = 16;
    verticesPerSegment = 16;
    glUniform1ui(0, segmentsPerNode);
    glUniform1ui(1, verticesPerSegment);
    indexCount = segmentsPerNode * verticesPerSegment * lastIdx * 6;

    GLuint vertexBuffer, normalBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, lastIdx * segmentsPerNode * verticesPerSegment * 4 * sizeof(float), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vertexBuffer);
    glGenBuffers(1, &elementBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, elementBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, lastIdx * segmentsPerNode * verticesPerSegment * 6 * sizeof(uint), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, elementBuffer);
    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, normalBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, lastIdx * segmentsPerNode * verticesPerSegment * 4 * sizeof(float), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, normalBuffer);

    glDispatchCompute(lastIdx, segmentsPerNode, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

void Tree::generateBoundingBoxes() {
    GLuint generateVerticesShader = loadComputeShader("shaders/compute/generatevertices.glsl");
    glUseProgram(generateVerticesShader);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, turtle->treeBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, turtle->treeBuffer);

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, lastIdx * 36 * 4 * sizeof(float), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vertexBuffer);

    glDispatchCompute(lastIdx, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

Tree::Tree(vec3 position, float treeScale, float branchAngle, float initialWidth, float widthDecay, int iterations, int type) {
    Model = translate(position) * scale(mat4(1.0f), vec3(treeScale));
	//
    lsystem = new Lsystem();
    lsystem->setAxiom("A");
    lsystem->addRule('A', "[&F[###^^L]!A]/////#[&F[###^^L]!A]///////#[&F[###^^L]!A]", 1.0f);
    lsystem->addRule('F', "S/////F", 1.0f);
    lsystem->addRule('S', "F[###^^L]", 1.0f);
    lsystem->addRule('[', "[", 1.0f);
    lsystem->addRule(']', "]", 1.0f);
    lsystem->addRule('L', "L", 1.0f);
    lsystem->addRule('&', "&", 1.0f);
    lsystem->addRule('^', "^", 1.0f);
    lsystem->addRule('/', "/", 1.0f);
    lsystem->addRule('!', "!", 1.0f);
    lsystem->addRule('#', "#", 1.0f);
    //lsystem->addRule('X', "FFFA", 1.0f);
    //lsystem->addRule('A', "FA", 0.6f);
    
    //lsystem->addRule('A', "F[!!^F/L][!!&F\\L]FA", 0.2f);
    //lsystem->addRule('A', "F[!!+F/L][!!-F\\L]FA", 0.2f);
    //lsystem->addRule('A', "F[!!^FA]!!&FA", 0.1f);
    //lsystem->addRule('A', "F[++!FL]/F[--!FL]A", 0.4f);
    //lsystem->addRule('L', "!F/F[+/L]F[-\\L]/A", 1.0f);
    //lsystem->addRule('F', "F+^F", 1.0f);
	//lsystem->setAxiom("X");
    lsystem->loadProductionsBuffer();
    lsystem->iterate(iterations);

	turtle = new Turtle(initialWidth, widthDecay, PI * branchAngle / 180);
	turtle->buildGPU(lsystem->inputBuffer, 6);
    
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, turtle->treeBuffer);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint), &lastIdx);
    //cout << "lastIdx: " << lastIdx << "\n";
    
    //generateBoundingBoxes();
    generateSplines();

    generateLeafVertexArray();

    //printing
    /*
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, elementBuffer);
    vector<uint> bufferdata;
    bufferdata.resize(100);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 100 * sizeof(uint), bufferdata.data());
    cout << "productions buffer: \n";
    for (uint c : bufferdata) {
        cout << c << ", ";
    } 
    cout << "\n";
    */
}

void Tree::render(GLuint shader, mat4 VP, vec3 camPos, GLuint leafShader) {
    mat4 MVP = VP * Model;
    mat4 ModelInv = inverse(Model);

    //leaves
    glBindVertexArray(leafVertexArray);
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
    glUniformMatrix4fv(4, 1, GL_FALSE, &ModelInv[0][0]);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, turtle->treeBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, turtle->treeBuffer);
    glBindVertexArray(vertexArray);
    
    //normal rendering (splines)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    //raycasting
    //glDrawArrays(GL_TRIANGLES, 0, 36 * lastIdx);
}