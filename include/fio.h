#ifndef fio
#define fio

extern int threading;
extern char *filename;

void print_header(void);

void print_to_csv(long long *descriptors, const char *instance, size_t threadNumber);

#endif
