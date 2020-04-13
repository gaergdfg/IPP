#ifndef ARRAY_UTIL_H
#define ARRAY_UTIL_H

#include <stdint.h>


#define MAX_HEIGHT (uint32_t)((1 << 32) - 2)


uint32_t* get_arr_32(uint32_t **arr, uint32_t x, uint32_t y) {
	return *(arr + y) + x;
}


uint64_t* get_arr_64(uint64_t **arr, uint32_t x, uint32_t y) {
	return *(arr + y) + x;
}


uint64_t get_array_index(uint64_t x, uint64_t y) {
	return y * MAX_HEIGHT + x + 1;
}


uint32_t get_array_x_from_index(uint64_t index) {
	return (index - 1) % MAX_HEIGHT;
}


uint32_t get_array_y_from_index(uint64_t index) {
	return (index - 1) / MAX_HEIGHT;
}

#endif /* ARRAY_UTIL_H */
