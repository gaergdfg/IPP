#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "memory_util.h"


/* possible ARRAY_FUNCTIONS_H */
#define MAX_HEIGHT (uint32_t)((1 << 32) - 2)


uint32_t* get_arr_32(uint32_t **arr, uint32_t x, uint32_t y) {
	return *(arr + y) + x;
}


uint64_t* get_arr_64(uint64_t **arr, uint32_t x, uint32_t y) {
	return *(arr + y) + x;
}


uint64_t get_array_index(uint64_t x, uint64_t y) {
	return y * MAX_HEIGHT + x + 1;
}


uint32_t get_array_x_from_index(uint64_t index) {
	return (index % MAX_HEIGHT) - 1;
}


uint32_t get_array_y_from_index(uint64_t index) {
	return index / MAX_HEIGHT;
}
/* possible ARRAY_FUNCTIONS_H */


typedef struct player {
	bool used_golden_move;
	uint64_t taken_fields;
	uint64_t available_fields;
	uint32_t occupied_areas;
} player_t;


player_t* player_new(uint64_t height, uint64_t width) {
	player_t *player = malloc(sizeof(player_t));
	if (!player) {
		return NULL;
	}

	player->used_golden_move = false;
	player->taken_fields = 0;
	player->available_fields = height * width;
	player->occupied_areas = 0;

	return player;
}


typedef struct gamma {
	uint32_t field_width;
	uint32_t field_height;
	uint32_t player_count;
	uint32_t max_player_areas;

	uint32_t* field; // plansza
	uint64_t* leader; // find&union lider tego pola
	player_t** players; // tablica z danymi graczy
} gamma_t;


bool check_gamma_exists(gamma_t* gamma) {
	return gamma ? true : false;
}


bool check_player_correct(gamma_t* gamma, uint32_t player) {
	return player > 0 && player <= gamma->player_count;
}


bool check_field_correct(gamma_t* gamma, uint32_t x, uint32_t y) {
	return x <= gamma->field_width && y <= gamma->field_height;
}


bool check_field_exists(gamma_t *gamma, uint32_t x, uint32_t y, uint32_t dir) {
	if (dir == 0) {
		// up, [y - 1][x]
		return y > 0;
	}
	if (dir == 1) {
		// right, [y][x + 1]
		return x + 1 <= gamma->field_width;
	}
	if (dir == 2) {
		// down, [y + 1][x]
		return y + 1 <= gamma->field_height;
	}
	// left, [y][x - 1]
	return x > 0;
}


uint32_t get_neighbour_count(
	gamma_t *gamma,
	uint32_t player,
	uint32_t x,
	uint32_t y
) {
	uint32_t neighbour_count = 0;

	if ( // up
		check_field_exists(gamma, x, y, 0) &&
		*get_arr_32(gamma->field, x, y - 1) == player
	) {
		neighbour_count++;
	}
	if ( // right
		check_field_exists(gamma, x, y, 1) &&
		*get_arr_32(gamma->field, x + 1, y) == player
	) {
		neighbour_count++;
	}
	if ( // down
		check_field_exists(gamma, x, y, 2) &&
		*get_arr_32(gamma->field, x, y + 1) == player
	) {
		neighbour_count++;
	}
	if ( // left
		check_field_exists(gamma, x, y, 3) &&
		*get_arr_32(gamma->field, x - 1, y) == player
	) {
		neighbour_count++;
	}

	return neighbour_count;
}


uint64_t find_leader(gamma_t *gamma, uint32_t x, uint32_t y) {
	uint64_t curr_leader = *get_arr_64(gamma->leader, x, y);
	uint64_t array_index = get_array_index(x, y);
	if (curr_leader == array_index) {
		return curr_leader;
	}

	uint32_t leader_x = get_array_x_from_index(curr_leader);
	uint32_t leader_y = get_array_y_from_index(curr_leader);
	uint64_t new_leader = find_leader(gamma, leader_x, leader_y);
	*get_arr_64(gamma->leader, x, y) = new_leader;

	return new_leader;
}


gamma_t* gamma_new(
	uint32_t width,
	uint32_t height,
	uint32_t players,
	uint32_t areas
) {
	if (width == 0 || height == 0 || players == 0 || areas == 0) {
		// consider: [areas] > [width] * [height]
		return NULL;
	}

	gamma_t *gamma = malloc(sizeof(gamma_t));
	if (!gamma) {
		return NULL;
	}

	gamma->field_width = width;
	gamma->field_height = height;
	gamma->player_count = players;
	gamma->max_player_areas = areas;

	// is it necessary to initiate it with zeros?
	gamma->field = allocate_2d_array(height, width);
	if (!gamma->field) {
		return NULL;
	}

	// is it necessary to initiate it with zeros?
	gamma->leader = allocate_2d_array(height, width);
	if (!gamma->leader) {
		return NULL;
	}

	gamma->players = malloc(players * sizeof(player_t));
	if (!gamma->players) {
		return NULL;
	}
	
	for (int i = 0; i < players; i++) {
		gamma->players[i] = player_new(height, width);
		if (!gamma->players[i]) {
			return NULL;
		}
	}

	return gamma;
}


void gamma_delete(gamma_t *g) {
	if (!check_gamma_exists(g)) {
		return;
	}
	
	free_2d_array(g->field, g->player_count);
	free_2d_array(g->leader, g->player_count);

	for (int i = 0; i < g->player_count; i++) {
		free(g->players[i]);
	}
	free(g->players);

	free(g);
}


void manage_leader(
	gamma_t *gamma, 
	uint32_t x,
	uint32_t y,
	uint32_t new_x,
	uint32_t new_y
) {
	uint64_t leader_index = find_leader(gamma, new_x, new_y);
	uint32_t leader_x = get_array_x_from_index(leader_index);
	uint32_t leader_y = get_array_y_from_index(leader_index);
	*get_arr_64(gamma->leader, leader_x, leader_y) = get_array_index(x, y);
}


bool gamma_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
	if (
		!check_gamma_exists(g) ||
		!check_player_correct(g, player) ||
		!check_field_correct(g, x, y) ||
		*get_arr_32(g->field, x, y) != 0
		// check if player can take this field if its not adjacent to any of his other fields
	) {
		return false;
	}

	// sets fields owner
	*get_arr_32(g->field, x, y) = player;
	g->players[player - 1]->taken_fields++;

	// connects adjacent fields of the player into one area
	*get_arr_64(g->leader, x, y) = get_array_index(x, y);
	g->players[player - 1]->occupied_areas++;
	if (get_neighbour_count(g, player, x, y) > 0) {
		if ( // up
			check_field_exists(g, x, y, 0) &&
			*get_arr_32(g->field, x, y - 1) == player
		) {
			manage_leader(g, x, y, x, y - 1);
		}

		if ( // right
			check_field_exists(g, x, y, 1) &&
			*get_arr_32(g->field, x + 1, y) == player
		) {
			manage_leader(g, x, y, x + 1, y);
		}

		if ( // down
			check_field_exists(g, x, y, 2) &&
			*get_arr_32(g->field, x, y + 1) == player
		) {
			manage_leader(g, x, y, x, y + 1);
		}

		if ( // left
			check_field_exists(g, x, y, 3) &&
			*get_arr_32(g->field, x - 1, y) == player
		) {
			manage_leader(g, x, y, x - 1, y);
		}
	}
	g->players[player - 1]->occupied_areas -=
		get_neighbour_count(g, player, x, y);

	// manage all players available fields

	return true;
}


bool gamma_golden_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
	if (
		!check_gamma_exists(g) ||
		!check_player_correct(g, player) ||
		!check_field_correct(g, x, y) ||
		*get_arr_32(g->field, x, y) == 0 ||
		*get_arr_32(g->field, x, y) == player
	) {
		return false;
	}

	// free the field and use gamma_move(), gg ez
	// if not possible, write undo_gamma_golden_move()

	return true;
}


uint64_t gamma_busy_fields(gamma_t *g, uint32_t player) {
	if (!check_gamma_exists(g) || !check_player_correct(g, player)) {
		return 0;
	}

	return g->players[player - 1]->taken_fields;
}


uint64_t gamma_free_fields(gamma_t *g, uint32_t player) {
	if (!check_gamma_exists(g) || !check_player_correct(g, player)) {
		return 0;
	}

	return g->players[player]->available_fields;
}


bool gamma_golden_possible(gamma_t *g, uint32_t player) {
	if (!check_gamma_exists(g) || !check_player_correct(g, player)) {
		return false;
	}

	bool res = !g->players[player - 1]->used_golden_move;
	if (!res) {
		return res;
	}
	
	for (int i = 0; i < g->player_count; i++) {
		if (i != player - 1) {
			res |= g->players[i]->taken_fields > 0;
		}
	}

	return res;
}


char* gamma_board(gamma_t *g) {
	// do stuff

	return "";
}
