/** @file
 * Interfejs modulu zajmujacego sie wykonywaniem polecen przekazywanych na
 * wejsciu
 * 
 * @author Piotr Prabucki <pp418377@students.mimuw.edu.pl>
 */


#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H


#include "gamma.h"
#include "parser.h"


/**
 * @brief Wykonuje polecenie @p command.
 * Zmienia stan gry w @p gamma stosownie do wydanego polecenia.
 * 
 * @param[in] command   : wskaznik na strukture przechowujaca polecenie
 * @param[in,out] gamma : wskaznik na wskaznik na strukture przechowujaca
 *                        dane gry
 * @param[in] line      : numer linii, w ktorej zostalo wywolane polecenie
 */
void execute_command(
	command_t *command,
	gamma_t **gamma,
	int *game_state,
	int line
);


#endif /* COMMAND_HANDLER_H */