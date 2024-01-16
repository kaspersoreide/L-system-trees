#include "glutils.h"

void createFramebuffer(uint* framebuffer, uint* texture, int width, int height, uint type, uint filter, uint wrap) {
	//gen texture
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, type, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

	//gen depthbuffer
    uint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	//gen framebuffer object
	glGenFramebuffers(1, framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	//unbind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint vertexArray2x3f(const vector<vec3>& vertices) {
	GLuint VBO, VAO;
	glCreateBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//writing all the vertices into the buffer
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(vec3) * vertices.size(),
		&vertices[0][0],
		GL_STATIC_DRAW
	);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		6 * sizeof(float),
		(void*)0
	);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		6 * sizeof(float),
		(void*)(3 * sizeof(float))
	);

	return VAO;
}

GLuint vertexArray1x3f(const vector<vec3>& vertices) {
	GLuint VBO, VAO;
	glCreateBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//writing all the vertices into the buffer
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(vec3) * vertices.size(),
		&vertices[0][0],
		GL_STATIC_DRAW
	);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		3 * sizeof(float),
		(void*)0
	);

	return VAO;
}

GLuint vertexArray1x2f(const vector<vec2>& vertices) {
	GLuint VBO, VAO;
	glCreateBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//writing all the vertices into the buffer
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(vec2) * vertices.size(),
		&vertices[0][0],
		GL_STATIC_DRAW
	);

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

	return VAO;
}

template<class T> GLuint loadVertexAttrib(vector<T> vertices, uint VAO, uint index) {
	GLuint VBO;
	glCreateBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//writing all the vertices into the buffer
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(T) * vertices.size(),
		vertices.data(),
		GL_STATIC_DRAW
	);

	glBindVertexArray(VAO);
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(
		index,
		sizeof(T) / sizeof(float),
		GL_FLOAT,
		GL_FALSE,
		sizeof(T),
		(void*)0
	);

	return VAO;
}

float getMilliseconds(chrono::steady_clock::time_point begin, chrono::steady_clock::time_point end) {
	return chrono::duration_cast<chrono::milliseconds>(end - begin).count();
}