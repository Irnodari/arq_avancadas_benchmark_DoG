#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <papi.h>
#include "papi_wrap.h"

long long results[REGCOUNT] = {0};

void papi_init(void){
	if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT){
		fprintf(stderr, "Unable to initialize papi library\n");
		exit(1);	
	}
	if (PAPI_thread_init(pthread_self) != PAPI_OK){
		fprintf(stderr, "Unable to initialize threading support\n");
		exit(1);	
	};
}

int papi_get_eventset(void){ //Returns eventset
	int events[REGCOUNT];
	int eventset = PAPI_NULL;
	events[0] = PAPI_TOT_CYC; //events[1] / events[0] = IPC
	events[1] = PAPI_TOT_INS;
//	events[2] = PAPI_L2_DCM; //events[2] / events[3] = Data cache miss rate
//	events[3] = PAPI_L2_DCA;

	if (PAPI_create_eventset(&eventset) != PAPI_OK){
		fprintf(stderr, "Unable to create eventset\n");
		exit(1);
	}
	if (PAPI_add_events(eventset, events, REGCOUNT) != PAPI_OK){
		fprintf(stderr, "Unable to add events\n");	
		exit(1);
	}
	return eventset;
}

void papi_start(int eventset){
	if (PAPI_start(eventset) != PAPI_OK){
		fprintf(stderr, "Unable to start eventset\n");
		exit(1);	
	}
	return;
}

long long *papi_end(int *eventset){
	long long *rv, event_results[REGCOUNT];
	if (PAPI_stop(*eventset, event_results) != PAPI_OK){
		fprintf(stderr, "Unable to stop eventset\n");
		exit(1);	
	}
	rv = malloc(REGCOUNT * sizeof(long long));
	memcpy(rv, event_results, REGCOUNT * sizeof(long long));
	PAPI_cleanup_eventset(*eventset);
	PAPI_destroy_eventset(eventset);
	return rv;
}

void papi_register_thread(void){
	PAPI_register_thread();
}

void papi_unregister_thread(void){
	PAPI_unregister_thread();
}
