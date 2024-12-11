#include <stdio.h>
#include <sys/stat.h>
#include "papi_wrap.h"
#include "semaphore.h"
#include "fio.h"

int threading;
char *filename;
sem_t semaphore;

void print_header(void){
	mkdir("out", 0755);
	FILE *f = fopen("out/results.csv", "w");
	fprintf(f, "DATASET,THREADING,INSTANCE,");
	for (int i = 0; i < REGCOUNT; i++){
		fprintf(f, "%s", papi_event_names[i]);
		if (i != REGCOUNT - 1) fprintf(f, ",");
		else fprintf(f, "\n");	
	}
	sem_init(&semaphore, 0, 1);
	fclose(f);
}

void print_to_csv(long long *descriptors, const char *instance, size_t threadNumber){
	sem_wait(&semaphore);
	FILE *f = fopen("out/results.csv", "a");
	fprintf(f, "%s,%d,%s_%d,", filename,  threading, instance, (int)threadNumber);
	for (int i = 0; i < REGCOUNT; i++){
		fprintf(f, "%Ld", descriptors[i]);
		if (i != REGCOUNT - 1) fprintf(f, ",");
		else fprintf(f, "\n");	
	}
	sem_post(&semaphore);
	fclose(f);
	return;
}


