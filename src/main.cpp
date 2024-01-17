#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <iostream>
#include <GL/glew.h>
#include "glm/ext.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtc/quaternion.hpp"
#include "GLFW/glfw3.h"
#include <vector>
#include "glm/gtc/matrix_transform.hpp"
#include "glutils.h"
#include "lsystem.h"
#include "turtle.h"
#include "loadshaders.h"
#include "tree.h"

GLFWwindow* window;
float cameraAngle = PI / 2;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	switch(key) {
		case GLFW_KEY_A:
			cameraAngle += 0.05f;
			break;
		case GLFW_KEY_D:
			cameraAngle -= 0.05f;
			break;
		default:
			break;
	}
}

void init() {
	glfwInit();
	window = glfwCreateWindow(1280, 720, "<3", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glfwSetKeyCallback(window, keyCallback);
}

void runTests() {
}


int main() {
	init();
	runTests();

	
	float fov = PI * 80.0f / 180;
	mat4 Projection = perspective(fov, 16.0f / 9, 0.01f, 100.0f);
	

	GLuint treeShader = loadShaders("shaders/tree/vert.glsl", "shaders/tree/frag.glsl");
	
	
	
	Tree tree;
	glLineWidth(1.0f);
	
	while (!glfwWindowShouldClose(window)) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 View = lookAt(
			vec3(3 * cosf(cameraAngle), 2.0, 3 * sinf(cameraAngle)),
			vec3(0.0, 2.0, 0.0),
			vec3(0.0, 1.0, 0.0)
		);
		mat4 VP = Projection * View;
		glUseProgram(treeShader);
		glUniformMatrix4fv(0, 1, GL_FALSE, &VP[0][0]);

		glBindVertexArray(tree.VAO);
		glDrawArrays(GL_LINES, 0, tree.vertexCount);		

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	return 0;
}