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
		trees.push_back(Tree(plantingPos, 1.0f, 39.7f, 0.2, 0.97,8));
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


void test(bool parallel, int iterations, int measurements) {
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
			lsystem.iterateParallel(iterations);
		}
		else {
			lsystem.iterate(iterations);
		}
		auto end = chrono::steady_clock::now();
		times.push_back(getMilliseconds(begin, end));
		stringSize = lsystem.stringSize;
		glFinish();

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

int main() {
	init();
	//for (int i = 4; i <= 7; i++) {
	//	test(true, i, 10);
	//}
	GLuint treeShader = loadShaders("shaders/tree/raycasting/vert.glsl", "shaders/tree/raycasting/frag.glsl");
	GLuint leafShader = loadShaders("shaders/leaf/render/vert.glsl", "shaders/leaf/render/frag.glsl");
	GLuint simpleTreeShader = loadShaders("shaders/tree/basic/vert.glsl", "shaders/tree/basic/frag.glsl");

	//trees.push_back(Tree(vec3(0.0f, 0.0f, -8.0f), 2.0f, 22.7f, 0.2f, 0.97, 4, 0));
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