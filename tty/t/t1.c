#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

int main(void)
{
	unsigned long cycles = 0;
	struct termios tp1, tp2;
	char buffer[64];
	int i, c;
	int fc;

	printf("Type a space character to exit...\n");

	tcgetattr(0, &tp1);
	tcgetattr(0, &tp2);

	fc = fcntl(0, F_GETFL);

	tp2.c_iflag &= ~ICRNL;
	tp2.c_lflag &= ~ICANON;
	tp2.c_lflag &= ~ECHO;
	tp2.c_cc[VMIN ] = 1;
	tp2.c_cc[VTIME] = 0;
	tp2.c_cc[VINTR] = 0xFF;
	tp2.c_cc[VSUSP] = 0xFF;
	tp2.c_cc[VQUIT] = 0xFF;

	fcntl(0, F_SETFL, fc | O_NONBLOCK);
	tcsetattr(0, TCSANOW, &tp2);

	while(1) {
		buffer[0] = 0;

		c = read(0, buffer, sizeof(buffer));

		if (c > 0) {
			for (i = 0; i < c; i++) {
				printf("0x%02x ", (unsigned char)buffer[i]);
			}

			printf("\n");
			fflush(stdout);

			if (buffer[0] == 0x20)
				break;
		} else {
			cycles += 1;	
		}

		fflush(stdin);
	}

	tcsetattr(0,TCSANOW,&tp1);
	fcntl(0, F_SETFL, fc);

	printf("cycles: %lu\n", cycles);
	return 0;
}
