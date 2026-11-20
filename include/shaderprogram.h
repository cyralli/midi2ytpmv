#include <GL/gl.h>
#include <GL/glext.h>
#include "shader.h"

GLuint shaderProgram;
void ShaderHandling() {
	// create our shaders
	#include "default.vert.h"
	GLuint vertShader;
	CreateShader(GL_VERTEX_SHADER, (const char*)shaders_default_vert, shaders_default_vert_len, &vertShader);
	#include "default.frag.h"
	GLuint fragShader;
	CreateShader(GL_FRAGMENT_SHADER, (const char*)shaders_default_frag, shaders_default_frag_len, &fragShader);
	
	// lets put them in a program
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertShader);
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);

	#ifndef NDEBUG

		GLint linkSuccess;
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkSuccess);
		if (linkSuccess == GL_FALSE) {
			char log[512];
			glGetProgramInfoLog(shaderProgram, 512, NULL, log);
			fprintf(stderr, "ERROR: Linking shaders to program failed!\n%s", log);

			abort();
		}

	#endif

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
}
