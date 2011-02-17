#include <inttypes.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <linux/fb.h>

uint32_t color16[] = {
	0x1F,
	0x7E0,
	0xF800,
	0xF81F,
	0xFFE0,
	0x7FF,
};

uint32_t color32[] = {
	0x000000FF,
	0x0000FF00,
	0x00FF0000,
	0x00FF00FF,
	0x00FFFF00,
	0x0000FFFF,
};

void usage(char *name);

int main (int argc, char *argv[])
{
	struct fb_var_screeninfo var_info;
	uint32_t *mem;

	char *fbname = "/dev/fb";

	int bpp, fbdev, opt;
	int size, w, h;
	int i, j;

	while ((opt = getopt(argc, argv, "hd:")) != -1) {
		switch (opt) {
			case 'd':
				fbname = strdup(optarg);
				break;
			case 'h':
			default:
				usage(argv[0]);
				exit(0);
		}
	}

	if (0 > (fbdev = open (fbname, O_RDWR))) {
		perror("fbdev open");
		exit(-1);
	}

	if (0 > ioctl(fbdev, FBIOGET_VSCREENINFO, &var_info)) {
		perror("ioctl FBIOGET_VSCREENINFO");
		exit(-1);
	}

	bpp = var_info.bits_per_pixel;
	h = var_info.yres;
	w = var_info.xres;

	size = h*w*bpp/8;

	printf("framebuffer settings: %d x %d x %d\n", w, h, bpp);
	getchar();

	var_info.xoffset = 0;
	var_info.yoffset = 0;

	if (0 > ioctl(fbdev, FBIOPAN_DISPLAY, &var_info)) {
		perror("ioctl FBIOPAN_DISPLAY");
		exit(-1);
	}

	mem = (uint32_t *) mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fbdev, 0);

	if (mem == MAP_FAILED) {
		perror("mmap");
		exit(-1);
	}

	for(i = 0; i < h; i++ ){
		uint32_t color;
		int xsize;

		switch (bpp) {
			case 16:
				color = (color16[6*i/h] << 16) | (color16[6*i/h]);
				xsize = w/2;
				break;
			case 32:
				color = color32[6*i/h];
				xsize = w;
				break;
			default:
				printf("unsupported bpp: %d\n", bpp);
				goto out;
		}

		for(j = 0; j < xsize; j++) {
			*(mem + i*xsize + j) = color;
		}
	}

out:
	munmap(mem, size);
	close(fbdev);
	return 0;
}

void usage(char *name)
{
	printf("usage: %s [-h] [-d <fbdev>]\n", name);
	printf("\t-h: this help message\n");
	printf("\t-d <fbdev>: framebuffer device, default is /dev/fb\n");
}
