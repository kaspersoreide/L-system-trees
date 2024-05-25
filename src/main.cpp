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
//Camera camera;
vec3 playerPos(0.0f, 1.8f, 0.0f);
vec2 angles(0.0f);
bool moveRight, moveLeft, moveBack, moveForward;
vector<Tree> trees;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_W) {
		if (action == GLFW_PRESS) moveForward = true;
		if (action == GLFW_RELEASE) moveForward = false;
	}
	if (key == GLFW_KEY_A) {
		if (action == GLFW_PRESS) moveLeft = true;
		if (action == GLFW_RELEASE) moveLeft = false;
	}
	if (key == GLFW_KEY_S) {
		if (action == GLFW_PRESS) moveBack = true;
		if (action == GLFW_RELEASE) moveBack = false;
	}
	if (key == GLFW_KEY_D) {
		if (action == GLFW_PRESS) moveRight = true;
		if (action == GLFW_RELEASE) moveRight = false;
	}
	if (key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(window, true);
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		vec3 plantingPos = vec3(playerPos.x - sinf(angles.x), 0.0f, playerPos.z - cosf(angles.x));
		trees.push_back(Tree(plantingPos, 1.0f, 45.0f, 0.3, 0.97,8 ));
	}
	/*if (key == GLFW_KEY_E) {
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
	}*/
}

void cursorCallback(GLFWwindow* window, double xpos, double ypos) {
	angles.x = -xpos * 0.001f;
	angles.y = -ypos * 0.001f;
}

void init() {
	glfwInit();
	window = glfwCreateWindow(1280, 720, "<3", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetCursorPosCallback(window, cursorCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	cursorCallback(window, xpos, ypos);

	glewInit();
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1, 0.2, 0.3, 0.0);
	
	glfwSetKeyCallback(window, keyCallback);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void movePlayer() {
	float speed = 0.08;
	vec3 forward = -vec3(sinf(angles.x), 0.0f, cosf(angles.x));
	vec3 right = vec3(-forward.z, 0.0, forward.x);
	if (moveForward) {
		playerPos += speed * forward;
	}
	if (moveBack) {
		playerPos -= speed * forward;
	}
	if (moveLeft) {
		playerPos -= speed * right;
	}
	if (moveRight) {
		playerPos += speed * right;
	}
}

int main() {
	init();
	
	GLuint treeShader = loadShaders("shaders/tree/raycasting/vert.glsl", "shaders/tree/raycasting/frag.glsl");
	GLuint leafShader = loadShaders("shaders/leaf/render/vert.glsl", "shaders/leaf/render/frag.glsl");
	GLuint simpleTreeShader = loadShaders("shaders/tree/basic/vert.glsl", "shaders/tree/basic/frag.glsl");

	trees.push_back(Tree(vec3(0.0f, 0.0f, -8.0f), 2.0f, 22.7f, 0.2f, 0.97, 4, 0));
	mat4 Projection = perspective(1.2f, 16.0f / 9, 0.1f, 1000.0f);
	mat4 playerModel;

	while (!glfwWindowShouldClose(window)) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//camera.move();
		movePlayer();
		playerModel = translate(playerPos)
					* rotate(angles.x, vec3(0.0, 1.0, 0.0))
					* rotate(angles.y, vec3(1.0, 0.0, 0.0));

		mat4 VP = Projection * inverse(playerModel);
		//mat4 Model = translate(mat4(1.0f), vec3(0.0f, 0.0f, -8.0f));
		//mat4 MVP = VP * Model;
		for (Tree tree : trees) {
			tree.render(simpleTreeShader, VP, playerPos, leafShader);
		}
		

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	return 0;
}