#include <stdint.h>
#include <stdlib.h>


uint32_t** allocate_2d_array(int n, int m) {
	uint32_t **arr = (uint32_t**)malloc(n * sizeof(uint32_t*));
	if (!arr) {
		return NULL;
	}
	uint32_t *arr_data = malloc(n * m * sizeof(uint32_t));
	if (!arr_data) {
		return NULL;
	}
	for(int i = 0; i < n; i++) {
		(*arr)[i] = arr_data + i * m;
	}
  	return arr;
}


void free_2d_array(uint32_t **arr, uint32_t size) {
	for (int i = 0; i < size; i++) {
		free(arr[i]);
	}
	free(arr);
}
