#include "gamma.h"
#include "parser.h"
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
	int line = 1;
	command_t *command;

	while ((command = get_command())->command_type != EXIT) {
		// execute_command(command, line);
		printf("%s(%d, %d, %d, %d)\n", toString(command), command->args[0], command->args[1], command->args[2], command->args[3]);
		erase_command(command);
		line++;
	}

	erase_command(command);

	return 0;
}
