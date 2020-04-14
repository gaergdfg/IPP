#ifndef MEMORY_UTIL_H
#define MEMORY_UTIL_H


#include <stdint.h>
#include <stdlib.h>


uint32_t** allocate_2d_array_uint32(int n, int m);


uint64_t** allocate_2d_array_uint64(int n, int m);


void free_2d_array_uint32(uint32_t **arr);


void free_2d_array_uint64(uint64_t **arr);


#endif /* MEMORY_UTIL_H */
