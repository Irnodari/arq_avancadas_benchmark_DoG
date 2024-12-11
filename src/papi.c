#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <papi.h>
#include "papi_wrap.h"

const int papi_events[] = {PAPI_TOT_CYC, PAPI_TOT_INS, PAPI_L2_DCM, PAPI_L2_DCA}; 
const char* papi_event_names[] = {"TOT_CYC","TOT_INS","L2_DCM","L2_DCA"};


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
	memcpy(events, papi_events, REGCOUNT * sizeof(int));

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
