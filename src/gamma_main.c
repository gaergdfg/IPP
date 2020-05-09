#include "gamma.h"
#include "parser.h"
#include "command_handler.h"
#include <stdio.h>

char *toString(command_t *command) {
	if (command->command_type == NEW_GAME_BATCH) {
		return "NEW_GAME_BATCH";
	}
	if (command->command_type == NEW_GAME_INTERACTIVE) {
		return "NEW_GAME_INTERACTIVE";
	}
	if (command->command_type == MOVE) {
		return "MOVE";
	}
	if (command->command_type == GOLDEN_MOVE) {
		return "GOLDEN_MOVE";
	}
	if (command->command_type == BUSY_FIELDS) {
		return "BUSY_FIELDS";
	}
	if (command->command_type == FREE_FIELDS) {
		return "FREE_FIELDS";
	}
	if (command->command_type == GOLDEN_POSSIBLE) {
		return "GOLDEN_POSSIBLE";
	}
	if (command->command_type == BOARD) {
		return "BOARD";
	}
	return "ERROR";
}


int main() {
	int line = 0, game_state = 0;
	gamma_t *gamma;
	command_t *command;

	while ((command = get_command())->command_type != EXIT) {
		printf("%s(%d, %d, %d, %d)\n", toString(command), command->args[0], command->args[1], command->args[2], command->args[3]);
		execute_command(command, &gamma, &game_state, ++line);
		erase_command(command);
	}

	erase_command(command);

	return 0;
}
