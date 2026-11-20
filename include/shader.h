#include <GL/gl.h>
#include <GL/glext.h>

#include <assert.h>
#include <stdlib.h> // abort()
#include <stdio.h>

void CreateShader(GLenum shadertype, const char* shadersource, int length, GLuint* id) {
	*id = glCreateShader(shadertype);
	glShaderSource(*id, 1, &shadersource, &length);
	glCompileShader(*id);

	// check if compiling gave an error
	// im doing the same logic as assert but i need to print the error
	#ifndef NDEBUG 
	
		GLint compSuccess;
		glGetShaderiv(*id, GL_COMPILE_STATUS, &compSuccess);
		if (compSuccess == GL_FALSE) {
			char log[512];
			glGetShaderInfoLog(*id, 512, NULL, log);
			fprintf(stderr, "GLSL ERROR: %s\n", log);

			abort();
		}
	
	#endif
}
