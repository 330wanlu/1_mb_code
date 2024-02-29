#ifndef MEM_LEAK_TEST_H
#define MEM_LEAK_TEST_H
#include "mb_typedef.h"
void * test_malloc(size_t size);
void test_free(void * ptr);
void mem_leak_check_result(void);
int mem_leak_check_init(void);
void mem_leak_check_destroy(void);


#endif