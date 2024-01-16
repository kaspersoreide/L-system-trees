#pragma once
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <string>


/* Generates a basic shader program */
GLuint loadShaders(const char* vertex, const char* frag);
GLuint loadComputeShader(const char* compute);