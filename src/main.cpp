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
#include "test.h"

GLuint treeBuildingShader;
GLuint genLeafShader;
GLuint generateSplinesShader;
GLuint stringAssignShader;
GLuint sumShader;
GLuint productShader;
GLuint bigsumShader;
    

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
		trees.push_back(Tree(plantingPos, 1.0f, 36.0f, 0.4, 0.97, 6, 0.8));
	}
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
	treeBuildingShader = loadComputeShader("shaders/compute/simpleInterpret.glsl");
	genLeafShader = loadShaders("shaders/leaf/generate/vert.glsl", "shaders/leaf/generate/frag.glsl");
	generateSplinesShader = loadComputeShader("shaders/compute/generatesplines.glsl");
	stringAssignShader = loadComputeShader("shaders/compute/stringassign.glsl");
    sumShader = loadComputeShader("shaders/compute/scansum.glsl");
    productShader = loadComputeShader("shaders/compute/genproduct.glsl");
    bigsumShader = loadComputeShader("shaders/compute/scansumsum.glsl");
    
	GLuint treeShader = loadShaders("shaders/tree/raycasting/vert.glsl", "shaders/tree/raycasting/frag.glsl");
	GLuint leafShader = loadShaders("shaders/leaf/render/vert.glsl", "shaders/leaf/render/frag.glsl");
	GLuint simpleTreeShader = loadShaders("shaders/tree/basic/vert.glsl", "shaders/tree/basic/frag.glsl");

	trees.push_back(Tree(vec3(0.0f, 0.0f, -8.0f), 2.0f, 22.7f, 0.2f, 0.97, 6, 0.8));
	mat4 Projection = perspective(1.2f, 16.0f / 9, 0.1f, 1000.0f);
	mat4 playerModel;

	float times[100];
	int counter = 0;
	while (!glfwWindowShouldClose(window)) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		movePlayer();
		playerModel = translate(playerPos)
					* rotate(angles.x, vec3(0.0, 1.0, 0.0))
					* rotate(angles.y, vec3(1.0, 0.0, 0.0));

		mat4 VP = Projection * inverse(playerModel);

		for (Tree tree : trees) {
			tree.render(simpleTreeShader, VP, playerPos, leafShader);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	return 0;
}