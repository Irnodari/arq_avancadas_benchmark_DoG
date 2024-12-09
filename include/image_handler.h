#ifndef image_handler
#define image_handler

#include <png.h>
#include <stdint.h>

typedef struct image{
	size_t width, height;
	png_byte color_type;
	png_byte bit_depth;
	png_bytep *row_pointers;
	size_t rowsize;
}IMG;

IMG *read_png_file(char *filename);

void write_png_file(char *filename, IMG *img);

#endif
