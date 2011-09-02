#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	FILE *cmd_pipe;
	int fd;

	char *line = "hello";
	double frac = 0.5;
	int sec = 1;

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	fd = atoi(argv[1]);
	cmd_pipe = fdopen(fd, "wb");
	setlinebuf(cmd_pipe);


	while (sec++) {
		fprintf(cmd_pipe, "progress %f %d\n", frac, sec);
		fprintf(cmd_pipe, "ui_print %s\n", line);
	}
}
