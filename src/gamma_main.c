/** @file
 * Glowny plik programu
 * 
 * @author Piotr Prabucki <pp418377@students.mimuw.edu.pl>
 */


#include "gamma.h"
#include "parser.h"
#include "command_handler.h"


int main() {
	int line = 0, game_state = 0;
	gamma_t *gamma;
	command_t *command;

	while ((command = get_command())->command_type != EXIT) {
		execute_command(command, &gamma, &game_state, ++line);
		erase_command(command);

		if (game_state == 2) {
			gamma_delete(gamma);
			return 0;
		}
	}

	if (game_state) {
		gamma_delete(gamma);
	}
	erase_command(command);

	return 0;
}
