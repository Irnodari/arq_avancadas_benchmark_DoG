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

#define MINTHREADS 31
#define MAXTHREADS 32

const char *imgnametemplate = "img/img%d.png";
const char *imgoutnametemplate = "out/img%d%s.png";
int main(int argc, char **argv){
	IMG *img, *res;
	char fname[128];
	int i = 1;
	filename = fname;
	papi_init();
	print_header();
	do{
		sprintf(fname, imgnametemplate, i);
		img = read_png_file(fname);
		if (img != NULL){
			threading = i;
//			for (int j = MINTHREADS; j <= MAXTHREADS; j++){
				res = DoG(img, DEVIATION, KERNEL_SIZE, KERNEL_SIZE_2, THAO, DEVIATION_SCALER, THRESHHOLD, 32, 'l');
				destroy_png(res);
				res = DoG(img, DEVIATION, KERNEL_SIZE, KERNEL_SIZE_2, THAO, DEVIATION_SCALER, THRESHHOLD, 32, 'c');
//			}
			sprintf(fname, imgoutnametemplate, i, "_out");
			write_png_file(fname, res);
			destroy_png(res);
			destroy_png(img);
		}
		i++;
	}while (img);

	return 0;
}
