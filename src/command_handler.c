/**
 * @file Implementacja modulu zajmujacego sie wykonywaniem polecen
 * przekazywanych na wejsciu.
 * 
 * @author Piotr Prabucki <pp418377@students.mimuw.edu.pl>
 */


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
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
) {
	int arg0 = command->args[0];
	int arg1 = command->args[1];
	int arg2 = command->args[2];
	int arg3 = command->args[3];

	switch (command->command_type) {
		case NEW_GAME_BATCH:
			if (*game_state > 0) {
				printf("ERROR1 %d\n", line);
				break;
			}
			*gamma = gamma_new(arg0, arg1, arg2, arg3);
			if (!(*gamma)) {
				printf("ERROR2 %d\n", line);
				break;
			} else {
				*game_state = 1;
				printf("OK %d\n", line);
			}
			break;
		case NEW_GAME_INTERACTIVE:
			if (*game_state > 0) {
				printf("ERROR %d\n", line);
				break;
			}
			*gamma = gamma_new(arg0, arg1, arg2, arg3);
			if (!(*gamma)) {
				*game_state = 2;
				printf("ERROR %d\n", line);
				break;
			}
			break;
		case MOVE:
			printf("%d\n", gamma_move(*gamma, arg0, arg1, arg2));
			break;
		case GOLDEN_MOVE:
			printf("%d\n", gamma_golden_move(*gamma, arg0, arg1, arg2));
			break;
		case BUSY_FIELDS:
			printf("%" PRIu64 "\n", gamma_busy_fields(*gamma, arg0));
			break;
		case FREE_FIELDS:
			printf("%" PRIu64 "\n", gamma_free_fields(*gamma, arg0));
			break;
		case GOLDEN_POSSIBLE:
			printf("%d\n", gamma_golden_possible(*gamma, arg0));
			break;
		case BOARD:;
			char *board = gamma_board(*gamma);
			printf("%s", board);
			free(board);
			break;
		case ERROR:
			printf("ERROR %d\n", line);
			break;
		default:
			break;
	}
}
