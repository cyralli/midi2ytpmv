#ifndef VIDEODATA_H
#define VIDEODATA_H

#include "pipe.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <GL/gl.h> // including gl for THE FASTEST FFMPEG VIDEO TO RAW IMAGE IN GPU!!!!
#include <GL/glext.h>

typedef struct VideoData {
	unsigned int width;
	unsigned int height;
	char* fps; // string
	GLuint texArrayid;
	int frameamount;
} VideoData;

void GetVideoInfo(int fd[2]);
void GetVideoFrames(int fd[2]);

// temporary for GetVideoInfo
VideoData tempVdata;

VideoData GetVideoData(char* filepath) {
	// We need to create a pipe with FFMpeg for this

	// ffprobe will get info from the video
	char* argvVideoInfo[] = {
		"ffprobe", "-v", "error", 
		"-select_streams", "v:0",
		"-count_frames",
		"-of", "default=noprint_wrappers=1:nokey=1",
		"-show_entries", "stream=r_frame_rate,width,height,nb_read_frames",
		filepath,
		NULL
	};
	CreatePipe(argvVideoInfo, &GetVideoInfo, READ, STDOUT_FILENO);

	// TIME TO GET THE FRAMES!!!
	// NOTE FOR LATER: you can use -vframes later on for limiting the amount of frames,
	// this can be used for ppl who dont have enough RAM
	printf("WARNING: The amount of VRAM that will be used for storing the source is ");
	printf("%d MB.\nPlease inform how many frames do you want to keep: ", tempVdata.width*tempVdata.height*4*tempVdata.frameamount/1000000);
	printf("If 0 or more than the original duration, then the original duration is kept. ");
	printf("\n\n(Detail: FPS is %s, Frames of video is %d) ", tempVdata.fps, tempVdata.frameamount);

	int USRvframes;
	scanf("%d", &USRvframes);
	if (USRvframes > 0 && USRvframes <= tempVdata.frameamount) tempVdata.frameamount = USRvframes;

	char vframesString[20];
	sprintf(vframesString, "%d", tempVdata.frameamount);

	char* argvVideoFrames[] = {
		"ffmpeg",
		"-i", filepath,
		"-pix_fmt", "rgba",
		"-vframes", vframesString,
		"-f", "rawvideo", "-",
		NULL
	};

	printf("\n\nFFMPEG pipe starting...\n");
	
	CreatePipe(argvVideoFrames, &GetVideoFrames, READ, STDOUT_FILENO);

	printf("Finished\n");

	return tempVdata;
}

void GetVideoInfo(int fd[2]) {
	int capacity = 64;
	char buffer[capacity];

	FILE* stream = fdopen(fd[0], "r");
	if (!stream) {
		fprintf(stderr, "Error opening stream for file descriptor.");
		exit(1);
		return;
	}
	int i = 0;
	while (fgets(buffer, capacity, stream) != NULL) {
		if (i > 3) {
			fprintf(stderr, "Expected ffprobe output to be 4 lines only. Something has gone wrong.\n");
			break;
		}
		switch (i) {
			case 0: tempVdata.width = atoi(buffer); break;
			case 1: tempVdata.height = atoi(buffer); break;
			case 2: tempVdata.fps = strdup(buffer); break;
			case 3: tempVdata.frameamount = atoi(buffer); break;
		}
		i++;
		printf("%s", buffer);
	}
	fclose(stream);
}

void GetVideoFrames(int fd[2]) {
	
	size_t totalpixels = tempVdata.width*tempVdata.height*4;
	void* pixels = malloc(totalpixels);

	// lets use a texture array for storing our textures
	glGenTextures(1, &tempVdata.texArrayid);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tempVdata.texArrayid);

	// alloc enough storage for the frames that about to come!! get ready!!
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, tempVdata.width, tempVdata.height, tempVdata.frameamount);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// loop until read gives 0 or error
	// also, we gotta use fdopen cause read() has a buffer limit as specified in the pipe() doc
	FILE* stream = fdopen(fd[0], "r");
	if (!stream) {
		fprintf(stderr, "Error opening stream for file descriptor.");
		exit(1);
		return;
	}
	
	int i = 0;
	size_t reading;
	while (1) {

		reading = fread(pixels, 1, totalpixels, stream);
		if (reading != totalpixels) break; // if didnt get exact size, were prob at EOB
		
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, tempVdata.width, tempVdata.height, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		++i;
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	free(pixels);
}

#endif
