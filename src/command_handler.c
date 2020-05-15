/** @file
 * Implementacja modulu zajmujacego sie wykonywaniem polecen
 * przekazywanych na wejsciu
 * 
 * @author Piotr Prabucki <pp418377@students.mimuw.edu.pl>
 */


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "gamma.h"
#include "parser.h"
#include "interactive_mode_handler.h"


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
	if (
		command->command_type != NEW_GAME_BATCH &&
		command->command_type != NEW_GAME_INTERACTIVE &&
		command->command_type != SKIP &&
		command->command_type != ERROR &&
		!(*game_state)
	) {
		fprintf(stderr, "ERROR %d\n", line);
		return;
	}

	int arg0 = command->args[0];
	int arg1 = command->args[1];
	int arg2 = command->args[2];
	int arg3 = command->args[3];

	switch (command->command_type) {
		case NEW_GAME_BATCH:
			if (*game_state > 0) {
				fprintf(stderr, "ERROR %d\n", line);
				break;
			}
			*gamma = gamma_new(arg0, arg1, arg2, arg3);
			if (!(*gamma)) {
				fprintf(stderr, "ERROR %d\n", line);
				break;
			} else {
				*game_state = 1;
				fprintf(stdout, "OK %d\n", line);
			}
			break;
		case NEW_GAME_INTERACTIVE:
			if (*game_state > 0) {
				fprintf(stderr, "ERROR %d\n", line);
				break;
			}
			*gamma = gamma_new(arg0, arg1, arg2, arg3);
			if (!(*gamma)) {
				fprintf(stderr, "ERROR %d\n", line);
				break;
			} else {
				*game_state = 2;
				run_interactive_mode(gamma ,arg0, arg1 + 1, arg2);
			}
			break;
		case MOVE:
			fprintf(stdout, "%d\n", gamma_move(*gamma, arg0, arg1, arg2));
			break;
		case GOLDEN_MOVE:
			fprintf(stdout, "%d\n", gamma_golden_move(*gamma, arg0, arg1, arg2));
			break;
		case BUSY_FIELDS:
			fprintf(stdout, "%" PRIu64 "\n", gamma_busy_fields(*gamma, arg0));
			break;
		case FREE_FIELDS:
			fprintf(stdout, "%" PRIu64 "\n", gamma_free_fields(*gamma, arg0));
			break;
		case GOLDEN_POSSIBLE:
			fprintf(stdout, "%d\n", gamma_golden_possible(*gamma, arg0));
			break;
		case BOARD:;
			char *board = gamma_board(*gamma);
			fprintf(stdout, "%s", board);
			free(board);
			break;
		case ERROR:
			fprintf(stderr, "ERROR %d\n", line);
			break;
		default:
			break;
	}
}
