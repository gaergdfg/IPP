/** @file
 * Interfejs modulu zawierajacego funkcje pomocnicze sluzace do zarzadzania
 * pamiecia przy tablicach dwuwymiarowych
 *
 * @author Piotr Prabucki <pp418377@students.mimuw.edu.pl>
 */


#ifndef MEMORY_UTIL_H
#define MEMORY_UTIL_H


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
uint32_t** allocate_2d_array_uint32(int n, int m);


/**
 * @brief Tworzy dwuwymiarowa tablice.
 * Alokuje pamiec na dwuwymiarowa tablice typu uint64_t.
 * 
 * @param[in] n : drugi wymiar tablicy
 * @param[in] m : pierwszy wymiar tablicy
 * 
 * @return Wskaznik na tablice lub NULL, gdy nie udalo sie zaalokowac pamieci.
 */
uint64_t** allocate_2d_array_uint64(int n, int m);


/**
 * @brief Zwalnia pamiec zaalokowana na tablice [arr].
 * Tablica jest typu uint32_t.
 * 
 * @param[in] arr   : tablica dwuwymiarowa, stworzona w
 *                    @ref allocate_2d_array_uint32
 */
void free_2d_array_uint32(uint32_t **arr);


/**
 * @brief Zwalnia pamiec zaalokowana na tablice [arr].
 * Tablica jest typu uint64_t.
 * 
 * @param[in] arr   : tablica dwuwymiarowa, stworzona w
 *                    @ref allocate_2d_array_uint64
 */
void free_2d_array_uint64(uint64_t **arr);


#endif /* MEMORY_UTIL_H */
