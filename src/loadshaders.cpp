#include "loadshaders.h"
#include <vector>

void loadShaderCodeFromFile(const char* file, GLuint shader) {
	std::ifstream source_file(file);

	std::string data;
	std::getline(source_file, data, '\0');

	const GLchar* shader_source(data.c_str());

	glShaderSource(shader, 1, &shader_source, NULL);
}

void checkShaderError(GLuint shader, const char* name) {
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		std::cout << "error in shader: " << name << '\n';
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		for (GLchar c : errorLog) {
			std::cout << c;
		}
		return;
	}
}

GLuint loadShaders(const char* vertex, const char* frag) {
	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint program = glCreateProgram();
	//Create shaders and shader program
	loadShaderCodeFromFile(vertex, vshader);
	loadShaderCodeFromFile(frag, fshader);

	glCompileShader(vshader);
	glCompileShader(fshader);

	checkShaderError(vshader, vertex);
	checkShaderError(fshader, frag);

	glAttachShader(program, vshader);
	glAttachShader(program, fshader);

	glLinkProgram(program);

	glDeleteShader(vshader);
	glDeleteShader(fshader);

	return program;
}

GLuint loadComputeShader(const char* compute) {
	GLuint cshader = glCreateShader(GL_COMPUTE_SHADER);
	GLuint program = glCreateProgram();
	loadShaderCodeFromFile(compute, cshader);
	glCompileShader(cshader);
	checkShaderError(cshader, compute);
	glAttachShader(program, cshader);
	glLinkProgram(program);
	glDeleteShader(cshader);
	return program;
}