#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "papi_wrap.h"
#include "image_handler.h"
#include "dog.h"
#include "image_processor.h"
#include "fio.h"


int main(int argc, char **argv){
	IMG *img, *res;
	papi_init();
	print_header();
	if (argc != 3){
		fprintf(stderr, "exec file_in.png file_out.png");
		return -1;
	};
	img = read_png_file(argv[1]);
	if (img == NULL){
		fprintf(stderr, "Unable to open %s.\n", argv[1]);	
		return -1;
	}
	res = DoG(img, DEVIATION, KERNEL_SIZE, KERNEL_SIZE_2, THAO, DEVIATION_SCALER, THRESHHOLD, 16);
	write_png_file(argv[2], res);
	destroy_png(res);
	destroy_png(img);

//	fprintf(stdout, "IPC: %f\nCMR: %f\n", results[0] * 1.0f / results[1], results[2] * 1.0f / results[3]);
	fprintf(stdout, "IPC: %f\nCycles: %Ld\n", results[1] * 1.0f / results[0], results[0]);
	return 0;
}
