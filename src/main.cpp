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

GLFWwindow* window;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	switch(key) {
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

	mat4 Projection = perspective(
		1.2f,
		static_cast<float>(16) / 9,
		0.1f,
		100.0f
	);
	/*
	Lsystem tree;
	tree.addRule('F', "F[+F]F[-F][F]", 0.5f);
	tree.addRule('F', "F[+F]F", 0.3f);
	tree.addRule('F', "F[-F]F", 0.2f);
	tree.addRule('[', "[", 1.0f);
	tree.addRule(']', "]", 1.0f);
	tree.addRule('+', "+", 1.0f);
	tree.addRule('-', "-", 1.0f);
	tree.setAxiom("F");
	int iterations = 5;
	//auto begin = chrono::steady_clock::now();
	tree.iterate(iterations);
	//auto end = chrono::steady_clock::now();
	//cout << "iterate " << iterations << " times GPU in milliseconds: " << getMilliseconds(begin, end) << "\n";
	//tree.iterateParallel(iterations);
	cout << tree.product << '\n';
	*/
	
	auto begin = chrono::steady_clock::now();
	//turtle.buildGPU(tree.inputBuffer);
	auto end = chrono::steady_clock::now();
	cout << "build geometry on Gpu in milliseconds: " << getMilliseconds(begin, end) << "\n";
	
	mat4 MVP(1.0f);
	MVP = scale(MVP, vec3(1.0f, 16.0f / 9, 1.0f));
	//MVP = rotate(MVP, float(std::_Pi / 2), vec3(0.0f, 0.0f, 1.0f));
	//MVP = translate(MVP, vec3(-0.4f, 0.0f, 0.0f));
	//MVP = translate(MVP, vec3(-0.1f, -0.1f, 0.0f));
	//Lsystem triangle;
	//triangle.addRule('B', "A-B-A", 1.0f);
	//triangle.addRule('A', "B+A+B", 1.0f);
	//triangle.setAxiom("A");
	//triangle.iterate(1);
	//triangle.iterateParallel(4);
	//Turtle turtle2(0.003f, 0.66667f, 1.047198f);
	//turtle2.buildGPU(triangle.inputBuffer);
	//turtle2.build(triangle.product);
	
	//for (vec2 v : turtle.vertices) {
	//	cout << v.x << ", " << v.y << "\n";
	//}
	vector<GLuint> VAOs;
	vector<int> VAOsizes;
	for (int i = 0; i < 4; i++) {
		Lsystem tree;
		tree.addRule('F', "F[+F]F[-F][F]", 0.5f);
		tree.addRule('F', "F[+F]F", 0.3f);
		tree.addRule('F', "F[-F]F", 0.2f);
		tree.setAxiom("F");
		tree.iterate(5);
		float angle = 20;
		Turtle turtle(0.02f, 0.2f, PI * angle / 180);
		turtle.state.pos = vec2(
			i * 0.4 - 0.6, 
			-0.5
 		);
		turtle.build(tree.product);
		VAOs.push_back(vertexArray1x2f(turtle.vertices));
		VAOsizes.push_back(turtle.vertices.size());
	}
	
		
	GLuint treeShader = loadShaders("shaders/tree/vert.glsl", "shaders/tree/frag.glsl");
	glUseProgram(treeShader);
	glUniformMatrix4fv(0, 1, GL_FALSE, &MVP[0][0]);
	
	glLineWidth(1.0f);
	
	while (!glfwWindowShouldClose(window)) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		for (int i = 0; i < VAOs.size(); i++) {
			glBindVertexArray(VAOs[i]);
			glDrawArrays(GL_LINES, 0, VAOsizes[i]);
		}
		

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	return 0;
}