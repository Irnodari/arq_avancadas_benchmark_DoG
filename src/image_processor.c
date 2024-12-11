#include <stdlib.h>
#include <math.h>
#include "image_handler.h"
#include "image_processor.h"

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

