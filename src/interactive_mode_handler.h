/** @file
 * Interfejs modulu obslugujacego tryb interaktywny gry gamma
 * 
 * @author Piotr Prabucki <pp418377@students.mimuw.edu.pl>
 */


#ifndef INTERACTIVE_MODE_HANDLER_H
#define INTERACTIVE_MODE_HANDLER_H


#include "gamma.h"


/**
 * @brief Obsluguje tryb interaktywny gry gamma dla danej planszy @p gamma.
 * Zmienia stan gry @p gamma zgodnie z poleceniami przekazywanymi programowi.
 * 
 * @param[in, out] gamma        : wskaznik na wskaznik na strukture
 *                                przechowujaca stan gry
 * @param[in] boundary_x        : maksymalna poprawna pozycja kursora na osi X
 * @param[in] boundary_y        : maksymalna poprawna pozycja kursora na osi Y
 * @param[in] max_player_number : liczba graczy w grze @p gamma
 */
void run_interactive_mode(
	gamma_t **gamma,
	long long boundary_x,
	long long boundary_y,
	uint32_t max_player_number
);


#endif /* INTERACTIVE_MODE_HANDLER_H */
