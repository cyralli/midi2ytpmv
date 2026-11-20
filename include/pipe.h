#ifndef CUSTOMPIPE_H
#define CUSTOMPIPE_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

// https://www.geeksforgeeks.org/c/c-program-demonstrate-fork-and-pipe/

typedef enum {
	READ,
	WRITE
} PipeIO;

void CreatePipe(char** argv, void (*parentprog)(int[2]), PipeIO IO, int std) {
	int fd[2]; // file descriptor, fd[0] is read, fd[1] is write over a pipe

	if (pipe(fd) == -1) {
		printf("Error opening pipe.\n");
		exit(1);
	}

	// fork
	pid_t pid = fork(); // fork is used to create child proc

	// fork will basically duplicate this program from the position that fork was called.
	// so its the reason why it runs the "if" that you see later on

	if (pid < 0) {
		printf("Error forking process.\n");
		exit(2);
	} else if (pid == 0) {
		// child process, ffmpeg
		close(fd[(int)IO]);
		
		if (dup2(fd[(int)(!IO)], std) < 0) {
			printf("Could not reopen file descriptor read end as stdin.\n");
			_exit(3);
		}
		// closing original fd since it was duplicated
		close(fd[(int)(!IO)]);
		
		execvp(argv[0], argv);
		// unreachable unless error
		perror("execvp");
		_exit(5);
		
	} else {
		close(fd[(int)(!IO)]);
		// parent, our program
		parentprog(fd);
		close(fd[(int)IO]);
		waitpid(pid, NULL, 0);
	}

	return;
}

#endif
