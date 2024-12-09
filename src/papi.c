#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <papi.h>
#include "papi_wrap.h"

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
	int eventset = PAPI_NULL;
	int events[4];

	events[0] = PAPI_TOT_CYC; //events[1] / events[0] = IPC
	events[1] = PAPI_TOT_INS;
	events[2] = PAPI_L2_DCM; //events[2] / events[3] = Data cache miss rate
	events[3] = PAPI_L2_DCA;

	if (PAPI_create_eventset(&eventset) != PAPI_OK){
		fprintf(stderr, "Unable to create eventset\n");
		exit(1);
	}
	if (PAPI_add_events(eventset, events, 4) != PAPI_OK){
		fprintf(stderr, "Unable to add events\n");	
		exit(1);
	}
	return eventset;
}

void papi_start(int eventset){
	if (PAPI_start(eventset) != PAPI_OK){
		fprintf(stderr, "Unable to start eventset");
		exit(1);	
	}
}

long long *papi_end(int *eventset){
	long long event_results[4];
	long long *retval;
	if (PAPI_stop(*eventset, event_results) != PAPI_OK){
		fprintf(stderr, "Unable to stop eventset\n");
		exit(1);	
	}
	PAPI_cleanup_eventset(*eventset);
	PAPI_destroy_eventset(eventset);
	*eventset = PAPI_NULL;
	retval = malloc(4 * sizeof(long long));
	memcpy(retval, event_results, 4 * sizeof(long long));
	return retval;
}

void papi_register_thread(void){
	PAPI_register_thread();
}

void papi_unregister_thread(void){
	PAPI_unregister_thread();
}
