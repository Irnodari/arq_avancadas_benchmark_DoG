#ifndef papi_wrap
#define papi_wrap

void papi_init(void);

int papi_get_eventset(void);

void papi_start(int eventset);

long long *papi_end(int *eventset); 

void papi_register_thread(void);

void papi_unregister_thread(void);
#endif
