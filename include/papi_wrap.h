#include <papi.h>

#ifndef papi_wrap
#define papi_wrap

#define REGCOUNT 4

extern long long results[REGCOUNT];

extern const int papi_events[];
extern const char* papi_event_names[];

void papi_init(void);

int papi_get_eventset(void);

void papi_start(int eventset);

long long *papi_end(int *eventset); 

void papi_register_thread(void);

void papi_unregister_thread(void);
#endif
