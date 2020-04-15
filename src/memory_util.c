#include <stdint.h>
#include <stdlib.h>


uint32_t** allocate_2d_array_uint32(uint64_t n, uint64_t m) {
	uint32_t **arr = (uint32_t**)calloc(n, sizeof(uint32_t*));
	if (!arr) {
		return NULL;
	}

	uint32_t *arr_data = calloc(n * m, sizeof(uint32_t));
	if (!arr_data) {
		return NULL;
	}

	for(uint32_t i = 0; i < n; i++) {
		arr[i] = arr_data + i * m;
	}

  	return arr;
}


uint64_t** allocate_2d_array_uint64(uint64_t n, uint64_t m) {
	uint64_t **arr = (uint64_t**)calloc(n, sizeof(uint64_t*));
	if (!arr) {
		return NULL;
	}

	uint64_t *arr_data = calloc(n * m, sizeof(uint64_t));
	if (!arr_data) {
		return NULL;
	}

	for(uint32_t i = 0; i < n; i++) {
		arr[i] = arr_data + i * m;
	}

  	return arr;
}


void free_2d_array_uint32(uint32_t **arr) {
	free(arr[0]);
	free(arr);
}


void free_2d_array_uint64(uint64_t **arr) {
	free(arr[0]);
	free(arr);
}
