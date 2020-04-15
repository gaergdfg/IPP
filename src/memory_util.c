/** @file
 * Implementacja funkcji pomocniczych sluzacych do zarzadzania pamiecia
 * przy tablicach dwuwymiarowych
 *
 * @author Piotr Prabucki <pp418377@students.mimuw.edu.pl>
 */


#include <stdint.h>
#include <stdlib.h>


/**
 * @brief Tworzy dwuwymiarowa tablice.
 * Alokuje pamiec na dwuwymiarowa tablice typu uint32_t.
 * 
 * @param[in] n : drugi wymiar tablicy
 * @param[in] m : pierwszy wymiar tablicy
 * 
 * @return Wskaznik na tablice lub NULL, gdy nie udalo sie zaalokowac pamieci.
 */
uint32_t** allocate_2d_array_uint32(uint64_t n, uint64_t m) {
	uint32_t **arr = (uint32_t**)calloc(n, sizeof(uint32_t*));
	if (!arr) {
		return NULL;
	}

	uint32_t *arr_data = calloc(n * m, sizeof(uint32_t));
	if (!arr_data) {
		free(arr);
		return NULL;
	}

	for(uint32_t i = 0; i < n; i++) {
		arr[i] = arr_data + i * m;
	}

  	return arr;
}


/**
 * @brief Tworzy dwuwymiarowa tablice.
 * Alokuje pamiec na dwuwymiarowa tablice typu uint64_t.
 * 
 * @param[in] n : drugi wymiar tablicy
 * @param[in] m : pierwszy wymiar tablicy
 * 
 * @return Wskaznik na tablice lub NULL, gdy nie udalo sie zaalokowac pamieci.
 */
uint64_t** allocate_2d_array_uint64(uint64_t n, uint64_t m) {
	uint64_t **arr = (uint64_t**)calloc(n, sizeof(uint64_t*));
	if (!arr) {
		return NULL;
	}

	uint64_t *arr_data = calloc(n * m, sizeof(uint64_t));
	if (!arr_data) {
		free(arr);
		return NULL;
	}

	for(uint32_t i = 0; i < n; i++) {
		arr[i] = arr_data + i * m;
	}

  	return arr;
}


/**
 * @brief Zwalnia pamiec zaalokowana na tablice [arr].
 * Tablica jest typu uint32_t.
 * 
 * @param[in] arr   : tablica dwuwymiarowa, stworzona w
 *                    @ref allocate_2d_array_uint32
 */
void free_2d_array_uint32(uint32_t **arr) {
	free(arr[0]);
	free(arr);
}


/**
 * @brief Zwalnia pamiec zaalokowana na tablice [arr].
 * Tablica jest typu uint64_t.
 * 
 * @param[in] arr   : tablica dwuwymiarowa, stworzona w
 *                    @ref allocate_2d_array_uint64
 */
void free_2d_array_uint64(uint64_t **arr) {
	free(arr[0]);
	free(arr);
}
