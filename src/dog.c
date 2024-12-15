#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include "papi_wrap.h"
#include "image_handler.h"
#include "image_processor.h"
#include "fio.h"

__attribute__((naked))
void flushLine(unsigned char *line, size_t lineSize){
	__asm__ volatile(
		"movq %rsi, %rcx;"
		".l0:"
		"clflush -4(%rdi, %rcx, 4);"
		"loop .l0;"
		"ret;"
	);
}

struct flushImgParams{
	IMG *img;
	int ID;
};

void *flushImgWrap(void *params){
	struct flushImgParams *parameters = params;
	int concurrency = get_nprocs();
	for (int i = parameters->ID; i < parameters->img->height; i += concurrency)
		flushLine(parameters->img->row_pointers[i], parameters->img->width);
	return NULL;
}

void flushImage(IMG *img){
	int len = get_nprocs();
	void *rv;
	pthread_t *threads;
	struct flushImgParams *params;
	params = malloc(len * sizeof(struct flushImgParams));
	threads = malloc(len * sizeof(pthread_t));
	for (size_t i = 0; i < len; i++){
		params[i].img = img;
		params[i].ID = i;	
	}
	for (size_t i = 0; i < len; i++){
		pthread_create(threads + i, NULL, flushImgWrap, params + i);	
	}
	for (size_t i = 0; i < len; i++)
		pthread_join(threads[i], &rv);
	free(params);
	free(threads);
	return;
}

struct foo_args{
	IMG *input; IMG *output; float **gaussian_weights; size_t kernel_size; int ID; int maxthreading;
};

void *blur_lines_wrapper(void *args){
	struct foo_args *argv = (struct foo_args*)args;
	int i, j;
	long long *result;
	papi_register_thread();
	int eventset = papi_get_eventset();
	papi_start(eventset);
	for (j = argv->ID; j < argv->input->height; j += argv->maxthreading){
		for (i = 0; i < argv->input->width; i++){
			blur_pixel(argv->input, argv->gaussian_weights, argv->kernel_size, i, j, &(argv->output)->row_pointers[j][4 * i]);
		}
	}
	result = papi_end(&eventset);
	char buffer[256];
	sprintf(buffer, "BLUR_LINES_HORIZONTAL_KERNEL%lu", argv->kernel_size);
	print_to_csv(result, buffer, argv->ID);
	free(result);
	papi_unregister_thread();
	return NULL;
}

void *blur_columns_wrapper(void *args){
	struct foo_args *argv = (struct foo_args*)args;
	int i, j;
	long long *result;
	papi_register_thread();
	int eventset = papi_get_eventset();
	papi_start(eventset);
	for (j = argv->ID; j < argv->input->width; j += argv->maxthreading){
		for (i = 0; i < argv->input->height; i++){
			blur_pixel(argv->input, argv->gaussian_weights, argv->kernel_size, j, i, &(argv->output->row_pointers[i][4 * j]));	
		}	
	}
	result = papi_end(&eventset);
	papi_unregister_thread();
	char buffer[256];
	sprintf(buffer, "BLUR_LINES_VERTICAL_KERNEL%lu", argv->kernel_size);
	print_to_csv(result, buffer, argv->ID);
	free(result);
	return NULL;
}

void blur_pixel_by_pixel(IMG *input, IMG *output, float **gaussian_weights, size_t kernel_size, size_t maxthreading, char orientation){
	int j;
	void *rv;
	pthread_t *threads;
	struct foo_args *func_args;
	threads = malloc(maxthreading * sizeof(pthread_t));
	func_args = malloc(maxthreading * sizeof(struct foo_args));
	for (j = 0; j < maxthreading; j++){
		func_args[j].gaussian_weights = gaussian_weights;
		func_args[j].input = input;
		func_args[j].output = output;
		func_args[j].kernel_size = kernel_size;
		func_args[j].ID = j;
		func_args[j].maxthreading = maxthreading;
	}
	for (j = 0; j < maxthreading; j++){
		pthread_create(threads + j, NULL, orientation == 'c' ? blur_columns_wrapper : blur_lines_wrapper, func_args + j);
	}
	for (j = 0; j < maxthreading; j++)
		pthread_join(threads[j], &rv);
	free(threads);
	free(func_args);
	return;
}

IMG *gaussian_blur(IMG *input, float stddev, size_t kernel_size, size_t maxthreading, char orientation){
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
	
	blur_pixel_by_pixel(input, retval, gaussian_weights, kernel_size, maxthreading, orientation);
	for (i = 0; i <= kernel_size; i++){
		free(gaussian_weights[i]);
	}
	free(gaussian_weights);


	return retval;
}

struct deriv_args{
	size_t ID;
	IMG *img;
	IMG *img2;
	float thao;
	float threshhold;
	size_t maxthreading;
};

void *deriv_img_horizontal_wrap(void *args){
	struct deriv_args *argv = (struct deriv_args*)args;
	png_bytep pixel1, pixel2;
	papi_register_thread();
	long long *result;
	int eventset = papi_get_eventset();
	papi_start(eventset);
	for (int j = argv->ID; j < argv->img->height; j += argv->maxthreading){
		for (int i = 0; i < argv->img->width; i++){
			pixel1 = &(argv->img->row_pointers[j][4 * i]);
			pixel2 = &(argv->img2->row_pointers[j][4 * i]);
			for (int k = 0; k < 4; k++){
				pixel1[k] = (png_byte)(pixel1[k] - argv->thao * pixel2[k]);
			}
			if (luminosity(pixel1[0], pixel1[1], pixel1[2]) < argv->threshhold){
				pixel1[0] = pixel1[1] = pixel1[2] = INVERT * 255;
			}
			else
				pixel1[0] = pixel1[1] = pixel1[2] = abs((INVERT - 1)) * 255;
			
		}	
	}
	result = papi_end(&eventset);
	print_to_csv(result, "DIFFERENTIATE_HORIZONTAL", argv->ID);
	free(result);
	papi_unregister_thread();
	return NULL;
}

void *deriv_img_vertical_wrap(void *args){
	struct deriv_args *argv = (struct deriv_args*)args;
	png_bytep pixel1, pixel2;
	papi_register_thread();
	long long *result;
	int eventset = papi_get_eventset();
	papi_start(eventset);
	for (int j = argv->ID; j < argv->img->width; j += argv->maxthreading){
		for (int i = 0; i < argv->img->height; i++){
			pixel1 = &(argv->img->row_pointers[i][4 * j]);
			pixel2 = &(argv->img2->row_pointers[i][4 * j]);
			for (int k = 0; k < 4; k++){
				pixel1[k] = (png_byte)(pixel1[k] - argv->thao * pixel2[k]);
			}
			if (luminosity(pixel1[0], pixel1[1], pixel1[2]) < argv->threshhold){
				pixel1[0] = pixel1[1] = pixel1[2] = INVERT * 255;
			}
			else
				pixel1[0] = pixel1[1] = pixel1[2] = abs((INVERT - 1)) * 255;
			
		}	
	}
	result = papi_end(&eventset);
	print_to_csv(result, "DIFFERENTIATE_VERTICAL", argv->ID);
	free(result);
	//papi_unregister_thread();
	return NULL;
}


void deriv_img(IMG *img, IMG *img2, float thao, float threshhold, size_t maxthreading, char orientation){
	pthread_t *threads;
	struct deriv_args *args;
	threads = malloc(maxthreading * sizeof(pthread_t));
	args = malloc(maxthreading * sizeof(struct deriv_args));
	void *ptr;

	for (int i = 0; i < maxthreading; i++){
		args[i].ID = i;
		args[i].img = img;
		args[i].img2 = img2;
		args[i].thao = thao;
		args[i].threshhold = threshhold;
		args[i].maxthreading = maxthreading;
	}
	for (int i = 0; i < maxthreading; i++){
		pthread_create(threads + i, NULL, orientation == 'c' ? deriv_img_vertical_wrap : deriv_img_horizontal_wrap, args + i);	
	}
	for (int i = 0; i < maxthreading; i++){
		pthread_join(threads[i], &ptr);	
	}
	free(threads);
	free(args);
	return;
}

IMG *DoG(IMG *img, float stddev, int kernel_size, int kernel_size_2, float thao, float deviation_scaler, float threshhold, size_t maxthreading, char orientation){
	IMG *gauss1, *gauss2;
	flushImage(img);
	gauss1 = gaussian_blur(img, stddev, kernel_size, maxthreading, orientation);
	flushImage(img);
	gauss2 = gaussian_blur(img, stddev * deviation_scaler, kernel_size_2, maxthreading, orientation);
	flushImage(img);
	deriv_img(gauss1, gauss2, thao, threshhold, maxthreading, orientation);
	destroy_png(gauss2);
	return gauss1;
}
