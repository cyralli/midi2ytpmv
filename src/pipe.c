#include "pipe.h"
#include <unistd.h>

void WriteToFFMPEG(int fd[2]) {
	// example: convert raw rgba to png
	char image[100*100*4];
	for (int col; col < 100*100; ++col) {
		image[col*4] = 100; //r
		image[col*4+1] = 100; //g
		image[col*4+2] = col/40; //b
		image[col*4+3] = 255; //a
	}
	write(fd[1], image, 100*100*4);
}

int main(void) {
	char* arg[] = {
		"ffmpeg",
		"-y",
		"-f", "rawvideo",
		"-pix_fmt", "rgba", // pixel format for stdin
		"-s", "100x100", // size
		"-i", "-", // specify for ffmpeg to look in stdin
		"output.png",
		NULL};
	CreatePipe(arg, &WriteToFFMPEG, WRITE, STDIN_FILENO);
	return 0;
}
