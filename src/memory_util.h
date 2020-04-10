#ifndef MEMORY_UTIL_H
#define MEMORY_UTIL_H


#include <stdint.h>
#include <stdlib.h>


uint32_t** allocate_2d_array(int n, int m);


void free_2d_array(uint32_t **arr, uint32_t size);


#endif /* MEMORY_UTIL_H */
