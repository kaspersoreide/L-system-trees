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
#include "camera.h"

GLFWwindow* window;
Camera camera;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_E) {
		if (action == GLFW_PRESS) camera.rot[0] = true;
		if (action == GLFW_RELEASE) camera.rot[0] = false;
	}
	if (key == GLFW_KEY_Q) {
		if (action == GLFW_PRESS) camera.rot[1] = true;
		if (action == GLFW_RELEASE) camera.rot[1] = false;
	}
	if (key == GLFW_KEY_LEFT) {
		if (action == GLFW_PRESS) camera.rot[2] = true;
		if (action == GLFW_RELEASE) camera.rot[2] = false;
	}
	if (key == GLFW_KEY_RIGHT) {
		if (action == GLFW_PRESS) camera.rot[3] = true;
		if (action == GLFW_RELEASE) camera.rot[3] = false;
	}
	if (key == GLFW_KEY_UP) {
		if (action == GLFW_PRESS) camera.rot[4] = true;
		if (action == GLFW_RELEASE) camera.rot[4] = false;
	}
	if (key == GLFW_KEY_DOWN) {
		if (action == GLFW_PRESS) camera.rot[5] = true;
		if (action == GLFW_RELEASE) camera.rot[5] = false;
	}

	if (key == GLFW_KEY_W) {
		if (action == GLFW_PRESS) camera.mov[0] = true;
		if (action == GLFW_RELEASE) camera.mov[0] = false;
	}
	if (key == GLFW_KEY_S) {
		if (action == GLFW_PRESS) camera.mov[1] = true;
		if (action == GLFW_RELEASE) camera.mov[1] = false;
	}
	if (key == GLFW_KEY_A) {
		if (action == GLFW_PRESS) camera.mov[2] = true;
		if (action == GLFW_RELEASE) camera.mov[2] = false;
	}
	if (key == GLFW_KEY_D) {
		if (action == GLFW_PRESS) camera.mov[3] = true;
		if (action == GLFW_RELEASE) camera.mov[3] = false;
	}
	if (key == GLFW_KEY_LEFT_SHIFT) {
		if (action == GLFW_PRESS) camera.mov[4] = true;
		if (action == GLFW_RELEASE) camera.mov[4] = false;
	}
	if (key == GLFW_KEY_LEFT_CONTROL) {
		if (action == GLFW_PRESS) camera.mov[5] = true;
		if (action == GLFW_RELEASE) camera.mov[5] = false;
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
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

int main() {
	init();
	
	GLuint treeShader = loadShaders("shaders/tree/boxvert.glsl", "shaders/tree/raycaster.glsl");
	
	Tree tree(25.7f, 0.05, 0.97, 5, 0);

	while (!glfwWindowShouldClose(window)) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera.move();
		mat4 VP = camera.getVP();
		mat4 Model = translate(mat4(1.0f), vec3(0.0f, 0.0f, -8.0f));
		mat4 MVP = VP * Model;
		tree.render(treeShader, Model, MVP, camera.getPos());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	return 0;
}