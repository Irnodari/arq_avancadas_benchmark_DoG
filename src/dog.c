#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include "threading.h"

#define EULER 2.72
#define DEVIATION 1.8
#define KERNEL_SIZE 2
#define KERNEL_SIZE_2 7
#define DEVIATION_SCALER 2
#define THAO 0.8
#define THRESHHOLD 0.80
#define INVERT 0

//Used this github page for the PNG reading part
// https://gist.github.com/niw/5963798

float luminosity(uint8_t R, uint8_t G, uint8_t B){
	float max = R, min = R;;
	if (G > max) max = G;
	if (B > max) max = B;
	max /= 255;
	if (G < min) min = G;
	if (B < min) min = B;
	min /= 255;
	return (max + min) / 2;
}

float saturation(uint8_t R, uint8_t G, uint8_t B){
	float lumen = luminosity(R, G, B);
	if (lumen > 0.99) return 0;
	float max = R, min = R;
	if (G > max) max = G;
	if (B > max) max = B;
	max /= 255;
	if (G < min) min = G;
	if (B < min) min = B;
	min /= 255;
	return fabs((max - min) / (1 - fabs(2 * lumen - 1)));
}

float hue(uint8_t R, uint8_t G, uint8_t B){
	float max = R, min = R;;
	if (G > max) max = G;
	if (B > max) max = B;
	max /= 255;
	if (G < min) min = G;
	if (B < min) min = B;
	min /= 255;
	if (R > G && R > B){
		return ((G - B) / 255.0) / (max - min);
	}
	else if (G > R && G > B){
		return 2.0 + ((B - R) / 255.0) / (max - min);
	}
	else return 4.0 + ((R - G) / 255.0) / (max - min);

	
}

typedef struct image{
	size_t width, height;
	png_byte color_type;
	png_byte bit_depth;
	png_bytep *row_pointers;
	size_t rowsize;
}IMG;

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

void blur_pixel(IMG *input, float **weights, size_t kernel_size, int coordx, int coordy, png_bytep pixel){
	int i, j, k;
	float acc[4] = {0.0};
	png_bytep row, px;
	for (i = coordx - kernel_size; i <= coordx + kernel_size; i++)
		for (j = coordy - kernel_size; j <= coordy + kernel_size; j++){
			row = input->row_pointers[j];
			px = &(row[4*i]);
			for (k = 0; k < 4; k++){
				if (i >= 0 && i < input->width && j >= 0 && j < input->height){
					acc[k] += px[k] * weights[abs(j - coordy)][abs(i - coordx)];
				}
			}
		}
	for (k = 0; k < 4; k++){
		pixel[k] = (char)floor(acc[k]);
	}
	pixel[3] = 255;
}

struct foo_args{
	IMG *input; IMG *output; float **gaussian_weights; size_t kernel_size; int ID;
};

void *blur_lines_wrapper(void *args){
	struct foo_args *argv = (struct foo_args*)args;
	int i, j;
	for (j = argv->ID; j < argv->input->height; j += MAXTHREADING){
		for (i = 0; i < argv->input->width; i++){
			blur_pixel(argv->input, argv->gaussian_weights, argv->kernel_size, i, j, &(argv->output)->row_pointers[j][4 * i]);
		}
	}
	return NULL;
}

void blur_pixel_by_pixel(IMG *input, IMG *output, float **gaussian_weights, size_t kernel_size){
	int j;
	void *rv;
	pthread_t threads[MAXTHREADING];
	struct foo_args func_args[MAXTHREADING];
	for (j = 0; j < MAXTHREADING; j++){
		func_args[j].gaussian_weights = gaussian_weights;
		func_args[j].input = input;
		func_args[j].output = output;
		func_args[j].kernel_size = kernel_size;
		func_args[j].ID = j;
	}
	for (j = 0; j < MAXTHREADING; j++){
		pthread_create(threads + j, NULL, blur_lines_wrapper, func_args + j);
	}
	for (j = 0; j < MAXTHREADING; j++)
		pthread_join(threads[j], rv);
}

IMG *gaussian_blur(IMG *input, float stddev, size_t kernel_size){
	IMG *retval;
	int i, j;
	float **gaussian_weights = malloc((kernel_size + 1) * sizeof(float*));
	for (i = 0; i <= kernel_size; i++){
		gaussian_weights[i] = malloc((kernel_size + 1) * sizeof(float));
	}
	for (i = 0; i <= kernel_size; i++){
		for (j = 0; j <= kernel_size; j++){
			gaussian_weights[i][j] = pow(EULER, -1.0 * (i * i + j * j) / (2.0 * stddev * stddev)) / (2 * M_PI * stddev * stddev);
		}
	}
	retval = malloc(sizeof(IMG));
	memcpy(retval, input, sizeof(IMG));
	retval->row_pointers = malloc(input->height * sizeof(png_bytep));
	for (i = 0; i < input->height; i++){
		retval->row_pointers[i] = malloc(retval->rowsize);
	}
	
//Now a wrapper for posterior pthread paralelism implementation
	blur_pixel_by_pixel(input, retval, gaussian_weights, kernel_size);

	return retval;
}

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

IMG *DoG(IMG *img, float stddev, int kernel_size, int kernel_size_2, float thao, float deviation_scaler, float threshhold){
	IMG *gauss1, *gauss2;
	int i, j, k;
	png_bytep pixel1, pixel2;
	gauss1 = gaussian_blur(img, stddev, kernel_size);
	gauss2 = gaussian_blur(img, stddev * deviation_scaler, kernel_size_2);
	for (j = 0; j < img->height; j++){
		for (i = 0; i < img->width; i++){
			pixel1 = &(gauss1->row_pointers[j][4 * i]);
			pixel2 = &(gauss2->row_pointers[j][4 * i]);
			for (k = 0; k < 4; k++){
				pixel1[k] = (png_byte)(pixel1[k] - thao * pixel2[k]);
			}
			if (luminosity(pixel1[0], pixel1[1], pixel1[2]) < threshhold){
				pixel1[0] = pixel1[1] = pixel1[2] = INVERT * 255;
			}
			else
				pixel1[0] = pixel1[1] = pixel1[2] = abs((INVERT - 1)) * 255;
		}
	}
	destroy_png(gauss2);
	return gauss1;
}

/*int main(int argc, char **argv){
	if (argc != 3){
		fprintf(stderr, "./gaussian_blur fin fout\n");
		return -1;
	}

	IMG *image = read_png_file(argv[1]);
	if (!image){
		fprintf(stderr, "Unable to open file\n");
		return -1;
	}

	IMG *output = DoG(image, DEVIATION, KERNEL_SIZE, KERNEL_SIZE_2, THAO, DEVIATION_SCALER, THRESHHOLD);

	write_png_file(argv[2], output);

	destroy_png(image);
	destroy_png(output);
	return 0;
}*/
