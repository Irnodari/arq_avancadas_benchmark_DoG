#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "papi_wrap.h"
//#include "dog.h"


int main(int argc, char **argv){
	int events;
	long long *results;
	papi_init();
	events = papi_get_eventset();
	papi_start(events);
	results = papi_end(&events);
	fprintf(stdout, "IPC: %f\nCMR: %f\n", results[0] * 1.0f / results[1], results[2] * 1.0f / results[3]);
	return 0;
}
