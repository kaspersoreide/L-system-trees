#include "tree.h"
#include <iostream>
#include "loadshaders.h"
#include <glm/gtx/transform.hpp>
#include <random>

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

void Tree::generateLeafTexture() {
    GLsizei res = 512;
    glGenTextures(1, &leafTexture);
	glBindTexture(GL_TEXTURE_2D, leafTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res, res, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//gen depthbuffer
    GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, res, res);

	//gen framebuffer object
    GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glBindTexture(GL_TEXTURE_2D, leafTexture);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, leafTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	//gen screen quad VAO
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	float data[] = {
		1.0, -1.0,
		1.0, 1.0,
		-1.0, 1.0,
		1.0, -1.0,
		-1.0, -1.0,
		-1.0, 1.0,
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), &data[0], GL_STATIC_DRAW);
    GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		2 * sizeof(float),
		(void*)0
	);
    GLuint genLeafShader = loadShaders("shaders/leaf/generate/vert.glsl", "shaders/leaf/generate/frag.glsl");
    glUseProgram(genLeafShader);
    glUniform1i(0, seed);
    glBindVertexArray(VAO);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, res, res);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
     glViewport(0, 0, 1280, 720);
    //glDeleteFramebuffers(1, &framebuffer);
    //glDeleteRenderbuffers(1, &depthBuffer);
    //glDeleteVertexArrays(1, &VAO);
    //glDeleteBuffers(1, &VBO);
}

void Tree::generateSplines() {
    GLuint generateSplinesShader = loadComputeShader("shaders/compute/generatesplines.glsl");
    glUseProgram(generateSplinesShader);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, turtle->treeBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, turtle->treeBuffer);

    
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
    
    /*
    lsystem->setAxiom("A");
    lsystem->addRule('A', "[&F[^^L]!A]/////[&F[^^L]!A]///////[&F[^^L]!A]", 1.0f);
    lsystem->addRule('F', "S/////F", 1.0f);
    lsystem->addRule('S', "F[^^L]", 1.0f);
    lsystem->addRule('[', "[", 1.0f);
    lsystem->addRule(']', "]", 1.0f);
    lsystem->addRule('L', "L", 1.0f);
    lsystem->addRule('&', "&", 1.0f);
    lsystem->addRule('^', "^", 1.0f);
    lsystem->addRule('/', "/", 1.0f);
    lsystem->addRule('!', "!", 1.0f);
    */
    
    lsystem->addRule('X', "FFFF!A", 1.0f);
    lsystem->addRule('A', "FA", 0.6f);
    lsystem->addRule('A', "F[!!+F/L][!!-F\\L]FA", 0.4f);
    lsystem->addRule('L', "!F/[+/L][-\\L]/A", 1.0f);
	lsystem->setAxiom("X");
    
    //lsystem->setAxiom("FL");
    //lsystem->addRule('L', "FL", 1.0f);

    lsystem->iterate(iterations);
    
    GLint bufferSize;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lsystem->inputBuffer);
    glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &bufferSize);
    cout << "string size: " << bufferSize / sizeof(uint) << "\n";


	turtle = new Turtle(initialWidth, widthDecay, PI * branchAngle / 180);
	turtle->buildGPU(lsystem->inputBuffer, 0.7f);
    
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, turtle->treeBuffer);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint), &lastIdx);
    //cout << "lastIdx: " << lastIdx << "\n";
    
    //generateBoundingBoxes();
    segmentsPerNode = 5;
    verticesPerSegment = 10;
    seed = std::rand();
    generateSplines();

    generateLeafVertexArray();
    generateLeafTexture();
    
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
    //mat4 ModelInv = inverse(Model);

    //leaves
    glBindVertexArray(leafVertexArray);
    glUseProgram(leafShader);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, turtle->leafModelsBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, turtle->leafModelsBuffer);
    glUniformMatrix4fv(0, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(1, 1, GL_FALSE, &Model[0][0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, leafTexture);
    glUniform1ui(glGetUniformLocation(leafShader, "leafTexture"), 0);
    glDrawArraysInstanced(GL_TRIANGLES, 0, leafVertexCount, lastLeafIdx + 1);

    //tree
    glUseProgram(shader);
	glUniformMatrix4fv(0, 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(1, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(3, 1, GL_FALSE, &VP[0][0]);
	glUniform3fv(2, 1, &camPos[0]);
    glUniform1i(4, seed);
    //glUniformMatrix4fv(4, 1, GL_FALSE, &ModelInv[0][0]);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, turtle->treeBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, turtle->treeBuffer);
    glBindVertexArray(vertexArray);
    //normal rendering (splines)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    //raycasting
    //glDrawArrays(GL_TRIANGLES, 0, 36 * lastIdx);    
}