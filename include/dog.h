#ifndef dog
#define dog
#include "image_handler.h"

IMG *DoG(IMG *img, float stddev, int kernel_size, int kernel_size_2, float thao, float deviation_scaler, float threshhold, size_t maxthreading, char orientation);

#endif
