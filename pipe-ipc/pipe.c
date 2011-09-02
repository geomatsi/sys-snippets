#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#define BINARY	"./xmit"

int main(void)
{
	char** args = malloc(sizeof(char*) * 5);
	char *command;

	char buffer[1024];
	FILE *from_child;
	pid_t pid, cpid;
	int pipefd[2];
	int status;

	pipe(pipefd);
	pid = fork();

	args[0] = BINARY;
	args[1] = (char *) malloc(20);
	sprintf(args[1], "%d", pipefd[1]);
	args[2] = NULL;

	if (pid == 0) {
		close(pipefd[0]);
		execv(BINARY, args);
		fprintf(stderr, "Error: can't run (%s)\n", strerror(errno));
		exit(-1);
	}

	close(pipefd[1]);
	from_child = fdopen(pipefd[0], "r");

	while (fgets(buffer, sizeof(buffer), from_child) != NULL) {
		command = strtok(buffer, " \n");

		if (command == NULL) {
			continue;
		} else if (strcmp(command, "progress") == 0) {
			char* fraction_s = strtok(NULL, " \n");
			char* seconds_s = strtok(NULL, " \n");

			float fraction = strtof(fraction_s, NULL);
			int seconds = strtol(seconds_s, NULL, 10);

			fprintf(stdout, "frac (%f) sec (%d)\n",
				fraction, seconds);
		} else if (strcmp(command, "ui_print") == 0) {
			char* str = strtok(NULL, "\n");
			if (str) {
				fprintf(stdout, "ui_print(%s)\n", str);
			} else {
				fprintf(stdout, "ui_print(NULL)\n");
			}
		} else {
			fprintf(stderr, "unknown command [%s]\n", command);
		}

	}

	fclose(from_child);
	waitpid(pid, &status, 0);

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
		fprintf(stderr, "Error: status (%d)\n", WEXITSTATUS(status));
	}

	return 0;
}
