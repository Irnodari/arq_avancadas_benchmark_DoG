#ifndef image_processor
#define image_processor
#include "image_handler.h"

#define EULER 2.72
#define DEVIATION 1.8
#define KERNEL_SIZE 2
#define KERNEL_SIZE_2 7
#define DEVIATION_SCALER 2
#define THAO 0.8
#define THRESHHOLD 0.80
#define INVERT 0

float luminosity(uint8_t R, uint8_t G, uint8_t B);

float saturation(uint8_t R, uint8_t G, uint8_t B);

float hue(uint8_t R, uint8_t G, uint8_t B);

#endif
