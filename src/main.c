#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "papi_wrap.h"
#include "image_handler.h"
#include "dog.h"
#include "image_processor.h"
#include "fio.h"

#define MINTHREADS 1
#define MAXTHREADS 72 //36 é a concorrência da minha CPU em particular

const char *imgnametemplate = "img/img%d.png";
const char *imgoutnametemplate = "out/img%d%s.png";
int main(int argc, char **argv){
	IMG *img, *res;
	char fname[128];
	int i = 1;
	filename = fname;
	papi_init();
	print_header();
	sprintf(fname, imgnametemplate, i);
	img = read_png_file(fname);
	do{
		if (img != NULL){
			for (int j = MINTHREADS; j <= MAXTHREADS; j++){
				threading = j;
				res = DoG(img, DEVIATION, KERNEL_SIZE, KERNEL_SIZE_2, THAO, DEVIATION_SCALER, THRESHHOLD, j, 'l');
				destroy_png(res);
				res = DoG(img, DEVIATION, KERNEL_SIZE, KERNEL_SIZE_2, THAO, DEVIATION_SCALER, THRESHHOLD, j, 'c');
			}
			write_png_file(fname, res);
			destroy_png(res);
		}
		i++;
	}while (i < 10);
	destroy_png(img);

	return 0;
}
