#include "pipe.h"
#include "videodata.h"
#include "miditoframe.h"
#include "shaderprogram.h"

#include <GL/gl.h>
#include <GL/glext.h>
#include <stdbool.h>

void RenderInPipe(int fd[2]);

NotesList tempNotes;
void Render(VideoData vData, NotesList notes) {

	tempVdata = vData;
	tempNotes = notes;

	// Setting up framebuffer to store frames

	GLuint FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	GLuint colorRBO;
	glGenRenderbuffers(1, &colorRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, colorRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, vData.width, vData.height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRBO);

	glViewport(0, 0, vData.width, vData.height);

	// implementing VAO to use glDrawElements
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	ShaderHandling(); // gives shaderProgram
	glUseProgram(shaderProgram);

	// connect the video (texture array) to our shader
	glUniform1f(glGetUniformLocation(shaderProgram, "video"), 0);

	// now we open the pipe

	char resolution[64];
	sprintf(resolution, "%dx%d", vData.width, vData.height);

	char* arg[] = {
		"ffmpeg", "-y",
		"-f", "rawvideo",
		"-pix_fmt", "rgba",
		"-s", resolution,
		"-r", vData.fps,
		"-i", "-",
		
		"-vf", "vflip",
		"-c:v", "libx264",
		"-vb", "2500k",
		"-c:a", "aac",
		"-ab", "200k",
		"-pix_fmt", "yuv420p",
		"output.mp4",
		NULL
	};

	printf("Starting FFMPEG pipe for writing.\n");
	
	CreatePipe(arg, RenderInPipe, WRITE, STDIN_FILENO);

	printf("Finished.\n");
}

void RenderInPipe(int fd[2]) {
	int framesize = tempVdata.width*tempVdata.height*4;
	uint8_t* pixels = malloc(framesize);

	int totalVideoLen = tempNotes.framestime[tempNotes.amount - 1] + tempVdata.frameamount;
	int sourcei = -1;
	int noteIndex = 0;
	bool reverse = false;
	// frame that we want to mirror: 2

	glBindTexture(GL_TEXTURE_2D_ARRAY, tempVdata.texArrayid);

	for (int i = 0; i < totalVideoLen; ++i) {
		
		if (i >= tempNotes.framestime[noteIndex]) { // while is being used to handle notes in the same time
			++noteIndex;
			sourcei = 0;
			reverse = false;
		}
		if (sourcei > tempVdata.frameamount) sourcei = -1;
		if (sourcei == 2) reverse = false;

		glUniform1i(glGetUniformLocation(shaderProgram, "time"), sourcei);
		glUniform1i(glGetUniformLocation(shaderProgram, "invertvid"), noteIndex%2);

		glClearColor(0.0, 0.0, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glReadPixels(0, 0, tempVdata.width, tempVdata.height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		write(fd[1], pixels, framesize);

		if (sourcei != -1) {
			if (!reverse) ++sourcei;
			//else --sourcei;
		}
	}

	free(pixels);
}
