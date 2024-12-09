#include <stdlib.h>
#include <png.h>
#include "image_handler.h"

//Used this github page for the PNG reading part
// https://gist.github.com/niw/5963798

void write_png_file(char *filename, IMG *img){
	int y;

  	FILE *fp = fopen(filename, "wb");
  	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  	png_infop info = png_create_info_struct(png);
  	setjmp(png_jmpbuf(png));

  	png_init_io(png, fp);
  	png_set_IHDR(
    		png,
    		info,
    		img->width, img->height,
    		8,
    		PNG_COLOR_TYPE_RGBA,
    		PNG_INTERLACE_NONE,
    		PNG_COMPRESSION_TYPE_DEFAULT,
    		PNG_FILTER_TYPE_DEFAULT
  	);
  	png_write_info(png, info);

  // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
  // Use png_set_filler().
  //png_set_filler(png, 0, PNG_FILLER_AFTER);

  	png_write_image(png, img->row_pointers);
  	png_write_end(png, NULL);

  	png_destroy_write_struct(&png, &info);
	fclose(fp);
}


void destroy_png(IMG *img){
	int i;
	for (i = 0; i < img->height; i++)
		free(img->row_pointers[i]);
	free(img->row_pointers);
	free(img);
}

IMG *read_png_file(char *filename){
	IMG *png;
	FILE *f = fopen(filename, "rb");
	if (f == NULL) return NULL;
	png_structp image = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop img_info = png_create_info_struct(image);
	setjmp(png_jmpbuf(image));
	png_init_io(image, f);
	png_read_info(image, img_info);

	png = malloc(sizeof(IMG));
	png->bit_depth = png_get_bit_depth(image, img_info);
	png->width = png_get_image_width(image, img_info);
	png->height = png_get_image_height(image, img_info);
	png->color_type = png_get_color_type(image, img_info);

	if(png->bit_depth == 16)
		png_set_strip_16(image);

	if(png->color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(image);

	if(png->color_type == PNG_COLOR_TYPE_GRAY && png->bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(image);

	if(png_get_valid(image, img_info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(image);

	if(png->color_type == PNG_COLOR_TYPE_RGB ||
	png->color_type == PNG_COLOR_TYPE_GRAY ||
	png->color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(image, 0xFF, PNG_FILLER_AFTER);

	if(png->color_type == PNG_COLOR_TYPE_GRAY ||
	png->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		  png_set_gray_to_rgb(image);

	png_read_update_info(image, img_info);

	png->row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * png->height);
	png->rowsize = png_get_rowbytes(image, img_info);
	for(int y = 0; y < png->height; y++) {
		png->row_pointers[y] = (png_byte*)malloc(png->rowsize);
	}

	png_read_image(image, png->row_pointers);

	fclose(f);

	png_destroy_read_struct(&image, &img_info, NULL);

	return png;
}


