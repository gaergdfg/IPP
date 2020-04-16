/** @file
 * Interfejs modulu zawierajacego funkcje pomocnicze do operowania na tablicach
 * dwuwymiarowych
 *
 * @author Piotr Prabucki <pp418377@students.mimuw.edu.pl>
 */


#ifndef ARRAY_UTIL_H
#define ARRAY_UTIL_H

#include <stdint.h>


/**
 * Stala, uzywana do obliczania indeksu pola w @ref get_arr_index.
 */
#define MAX_HEIGHT (uint32_t)((((uint64_t)1) << 32) - 2)


/**
 * @brief Zwraca wartosc arr[y][x].
 * [arr] jest typu uint32_t.
 * 
 * @param[in] arr   : tablica dwuwymiarowa, ktorej wartosc odczytujemy
 * @param[in] x     : drugi indeks tablicy
 * @param[in] y     : pierwszy indeks tablicy
 * 
 * @return wskaznik na arr[y][x]
 */
uint32_t* get_arr_32(uint32_t **arr, uint32_t x, uint32_t y);


/**
 * @brief Zwraca wartosc arr[y][x].
 * [arr] jest typu uint64_t.
 * 
 * @param[in] arr   : tablica dwuwymiarowa, ktorej wartosc odczytujemy
 * @param[in] x     : drugi indeks tablicy
 * @param[in] y     : pierwszy indeks tablicy
 * 
 * @return wskaznik na arr[y][x]
 */
uint64_t* get_arr_64(uint64_t **arr, uint32_t x, uint32_t y);


/**
 * Zwraca indeks reprezentujacy pole na potrzeby algorytmu Find & Union.
 * 
 * @param[in] x : wspolrzedna osi X pola
 * @param[in] y : wspolrzedna osi Y pola
 * 
 * @return Indeks reprezentujacy pole.
 */
uint64_t get_array_index(uint64_t x, uint64_t y);


/**
 * Zwraca wspolrzedna osi X bazujac na indeksie pola obliczonego w
 * @ref get_array_index.
 * 
 * @param[in] index : indeks pola, ktorego wspolrzedna osi X obliczamy
 * 
 * @return Wspolrzedna osi X pola o indeksie [index].
 */
uint32_t get_array_x_from_index(uint64_t index);


/**
 * Zwraca wspolrzedna osi X bazujac na indeksie pola obliczonego w
 * @ref get_array_index.
 * 
 * @param[in] index : indeks pola, ktorego wspolrzedna osi X obliczamy
 * 
 * @return Wspolrzedna osi X pola o indeksie [index].
 */
uint32_t get_array_y_from_index(uint64_t index);

#endif /* ARRAY_UTIL_H */
