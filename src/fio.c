#include <stdio.h>
#include <sys/stat.h>
#include "papi_wrap.h"
#include "fio.h"

void print_header(void){
	mkdir("out", 0755);
	FILE *f = fopen("out/results.csv", "w");

	fclose(f);
}

void print_to_csv(long long *descriptors, size_t threadNumber){
	FILE *f = fopen("out/results.csv", "a");

	fclose(f);
}


