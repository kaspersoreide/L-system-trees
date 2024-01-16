#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <chrono>
#define PI 3.14159265358979323846

using namespace std;
using namespace glm;

void createFramebuffer(uint* framebuffer, uint* texture, int width, int height, uint type, uint filter, uint wrap);

GLuint vertexArray2x3f(const vector<vec3>& vertices);

GLuint vertexArray1x3f(const vector<vec3>& vertices);

GLuint vertexArray1x2f(const vector<vec2>& vertices);


template<class T> GLuint loadVertexAttrib(vector<T> vertices, uint VAO, uint index);

float getMilliseconds(chrono::steady_clock::time_point begin, chrono::steady_clock::time_point end);