#include "test.h"
#include <iostream>
#include "loadshaders.h"

using namespace std;

void testLsystem(bool parallel, int iterations, int measurements) {
	int n = measurements;
	vector<float> times;
	int stringSize;
	for (int i = 0; i < n; i++) {
		Lsystem lsystem;
		lsystem.setAxiom("A");
    	lsystem.addRule('A', "[&F[^^L]!A]/////[&F[^^L]!A]///////[&F[^^L]!A]", 1.0f);
    	lsystem.addRule('F', "S/////F", 1.0f);
    	lsystem.addRule('S', "F[^^L]", 1.0f);
    	lsystem.addRule('[', "[", 1.0f);
    	lsystem.addRule(']', "]", 1.0f);
    	lsystem.addRule('L', "L", 1.0f);
    	lsystem.addRule('&', "&", 1.0f);
    	lsystem.addRule('^', "^", 1.0f);
    	lsystem.addRule('/', "/", 1.0f);
    	lsystem.addRule('!', "!", 1.0f);
		auto begin = chrono::steady_clock::now();
		if (parallel) {
			lsystem.iterateParallel(iterations, 0);
		}
		else {
			lsystem.iterate(iterations);
		}
        glFinish();
		auto end = chrono::steady_clock::now();
		times.push_back(getMilliseconds(begin, end));
		stringSize = lsystem.stringSize;
		//glFinish();
	}
	float sum = 0.0f;
	for (float t : times) {
		sum += t;
	}
	float avg = sum / n;
	float var = 0.0f;
	for (float t : times) {
		var += (t - avg) * (t - avg);
	}
	var /= n;
	float sd = sqrtf(var);
    std::cout << "time elapsed iterating " << iterations << " times: " << avg << " +- " << sd << "\n"; 
	std::cout << "string length: " << stringSize << "\n";
}

void testInterpreter(int iterations, int measurements, uint segmentsPerNode, uint verticesPerSegment) {
    Lsystem lsystem;
	lsystem.setAxiom("A");
    lsystem.addRule('A', "[&F[^^L]!A]/////[&F[^^L]!A]///////[&F[^^L]!A]", 1.0f);
    lsystem.addRule('F', "S/////F", 1.0f);
    lsystem.addRule('S', "F[^^L]", 1.0f);
    lsystem.addRule('[', "[", 1.0f);
    lsystem.addRule(']', "]", 1.0f);
    lsystem.addRule('L', "L", 1.0f);
    lsystem.addRule('&', "&", 1.0f);
    lsystem.addRule('^', "^", 1.0f);
    lsystem.addRule('/', "/", 1.0f);
    lsystem.addRule('!', "!", 1.0f);
    lsystem.iterateParallel(iterations, 0);
    int n = measurements;
	vector<float> times;
    GLuint generateSplinesShader = loadComputeShader("shaders/compute/generatesplines.glsl");
    Turtle t(0.2f, 0.98, PI * 40.0f / 180);
    
    t.buildGPU(lsystem.inputBuffer, 1.0f);
    for (int i = 0; i < n; i++) {
        
        glFinish();
        auto begin = chrono::steady_clock::now();


        
        uint lastIdx;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, t.treeBuffer);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint), &lastIdx);

        glUseProgram(generateSplinesShader);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, t.treeBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, t.treeBuffer);
        glUniform1ui(0, segmentsPerNode);
        glUniform1ui(1, verticesPerSegment);
        uint indexCount = segmentsPerNode * verticesPerSegment * lastIdx * 6;

        GLuint vertexBuffer, normalBuffer, elementBuffer, vertexArray;
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
        
        glFinish();
        auto end = chrono::steady_clock::now();
		times.push_back(getMilliseconds(begin, end));
        glDeleteVertexArrays(1, &vertexArray);
        glDeleteBuffers(1, &elementBuffer);
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &normalBuffer);
    }
    float sum = 0.0f;
	for (float t : times) {
		sum += t;
	}
	float avg = sum / n;
	float var = 0.0f;
	for (float t : times) {
		var += (t - avg) * (t - avg);
	}
	var /= n;
	float sd = sqrtf(var);
    std::cout << "time elapsed for interpreterer afer " << iterations << " iterations: " << avg << " +- " << sd << "\n"; 
	std::cout << "string length: " << lsystem.stringSize << "\n";
}  


void testLeafTexture(int res, int measurements, GLuint leafShader) {
    int n = measurements;
    vector<float> times;
    for (int i  = 0; i < n; i++) {
        auto begin = chrono::steady_clock::now();
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
        int leafVertexCount = positions.size();
        GLuint vertexBuffer, uvBuffer, leafVertexArray;
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


        //texture
        GLuint leafTexture;
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
        //GLuint genLeafShader = loadShaders("shaders/leaf/generate/vert.glsl", "shaders/leaf/generate/frag.glsl");
        glUseProgram(leafShader);
        glUniform1i(0, 0);
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
        auto end = chrono::steady_clock::now();
        times.push_back(getMilliseconds(begin, end));
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteTextures(1, &leafTexture);
        glDeleteRenderbuffers(1, &depthBuffer);
    }
    float sum = 0.0f;
	for (float t : times) {
		sum += t;
	}
	float avg = sum / n;
	float var = 0.0f;
	for (float t : times) {
		var += (t - avg) * (t - avg);
	}
	var /= n;
	float sd = sqrtf(var);
    std::cout << "time elapsed for generating leaf texture with resolution " << res << ": " << avg << " +- " << sd << "\n"; 
}