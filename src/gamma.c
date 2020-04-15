#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "array_util.h"
#include "memory_util.h"

/* DEBUGGING */
// #include <stdio.h>
/* DEBUGGING */


// axes offset values to iterate through adjecent fields on the field
// up, right, down, left
const uint32_t offset_x[4] = {0, 1, 0, -1};
const uint32_t offset_y[4] = {-1, 0, 1, 0};


typedef struct player {
	bool used_golden_move;
	uint64_t taken_fields;
	uint64_t available_fields_adjacent;
	uint64_t available_fields_far;
	uint32_t occupied_areas;
} player_t;


player_t* player_new(uint64_t height, uint64_t width) {
	player_t *player = malloc(sizeof(player_t));
	if (!player) {
		return NULL;
	}

	player->used_golden_move = false;
	player->taken_fields = 0;
	player->available_fields_adjacent = 0;
	player->available_fields_far = height * width;
	player->occupied_areas = 0;

	return player;
}


typedef struct g {
	uint32_t field_width;
	uint32_t field_height;
	uint32_t player_count;
	uint32_t max_player_areas;

	uint32_t** field; // plansza
	uint64_t** leader; // find&union lider tego pola
	player_t** players; // tablica z danymi graczy
} gamma_t;


/* DEBUGGING */
// void gamma_print(gamma_t *g) {
// 	printf("=== GAMMA PROPERTIES ===\n");
// 	printf("{ width: %d, height: %d, player_count: %d, max_areas: %d }\n", g->field_width, g->field_height, g->player_count, g->max_player_areas);

// 	printf("=== PLAYERS ===\n");
// 	for (uint32_t i = 0; i < g->player_count; i++) {
// 		printf(
// 			"#%d: { taken_fields: %ld, available_adjacent: %ld, available_far: %ld, areas: %d }\n",
// 			i + 1,
// 			g->players[i]->taken_fields,
// 			g->players[i]->available_fields_adjacent,
// 			g->players[i]->available_fields_far,
// 			g->players[i]->occupied_areas
// 		);
// 	}

// 	printf("=== FIELD ===\n");
// 	for (uint32_t y = 0; y < g->field_height; y++) {
// 		for (uint32_t x = 0; x < g->field_width; x++) {
// 			printf("%d ", *get_arr_32(g->field, x, y));
// 		}
// 		printf("\n");
// 	}

// 	printf("==========================================================================================\n");
// }
/* DEBUGGING */


/* checking if arguments are correct */
bool check_gamma_exists(gamma_t* g) {
	return g ? true : false;
}


bool check_player_correct(gamma_t* g, uint32_t player) {
	return player > 0 && player <= g->player_count;
}


bool check_field_correct(gamma_t* g, uint32_t x, uint32_t y) {
	return x < g->field_width && y < g->field_height;
}
/* checking if the arguments are correct */


// check if the field[y][x] exists in our game
bool check_field_exists(gamma_t *g, uint32_t x, uint32_t y, uint32_t dir) {
	if (dir == 0) {
		// up, [y - 1][x]
		return y > 0;
	}
	if (dir == 1) {
		// right, [y][x + 1]
		return x + 1 < g->field_width;
	}
	if (dir == 2) {
		// down, [y + 1][x]
		return y + 1 < g->field_height;
	}
	// left, [y][x - 1]
	return x > 0;
}


// returns the number of adjacent field that belong to [player]
uint32_t get_neighbour_count(
	gamma_t *g,
	uint32_t player,
	uint32_t x,
	uint32_t y
) {
	uint32_t neighbour_count = 0;

	for (int i = 0; i < 4; i++) {
		uint32_t new_x = x + offset_x[i];
		uint32_t new_y = y + offset_y[i];
		if (
			check_field_exists(g, x, y, i) &&
			*get_arr_32(g->field, new_x, new_y) == player
		) {
			neighbour_count++;
		}
	}

	return neighbour_count;
}


// finds the leader of the field
uint64_t find_leader(gamma_t *g, uint32_t x, uint32_t y) {
	uint64_t curr_leader = *get_arr_64(g->leader, x, y);
	uint64_t array_index = get_array_index(x, y);
	if (curr_leader == array_index) {
		return curr_leader;
	}

	uint32_t leader_x = get_array_x_from_index(curr_leader);
	uint32_t leader_y = get_array_y_from_index(curr_leader);
	uint64_t new_leader = find_leader(g, leader_x, leader_y);
	*get_arr_64(g->leader, x, y) = new_leader;

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

	gamma_t *g = malloc(sizeof(gamma_t));
	if (!g) {
		return NULL;
	}

	g->field_width = width;
	g->field_height = height;
	g->player_count = players;
	g->max_player_areas = areas;

	// do I have to initiate it with zeros?
	g->field = allocate_2d_array_uint32(height, width);
	if (!g->field) {
		free(g);
		return NULL;
	}

	// do I have to initiate it with zeros?
	g->leader = allocate_2d_array_uint64(height, width);
	if (!g->leader) {
		free_2d_array_uint32(g->field);
		free(g);
		return NULL;
	}

	g->players = malloc(players * sizeof(player_t));
	if (!g->players) {
		free_2d_array_uint32(g->field);
		free_2d_array_uint64(g->leader);
		free(g);
		return NULL;
	}
	
	for (uint32_t i = 0; i < players; i++) {
		g->players[i] = player_new(height, width);
		if (!g->players[i]) {
			for (long long j = i - 1; j >= 0; j--) {
				free(g->players[j]);
			}
			free(g->players);

			free_2d_array_uint32(g->field);
			free_2d_array_uint64(g->leader);
			free(g);
			return NULL;
		}
	}

	return g;
}


void gamma_delete(gamma_t *g) {
	if (!check_gamma_exists(g)) {
		return;
	}
	
	free_2d_array_uint32(g->field);
	free_2d_array_uint64(g->leader);

	for (uint32_t i = 0; i < g->player_count; i++) {
		free(g->players[i]);
	}
	free(g->players);

	free(g);
}


// sets the leader of the field[new_y][new_x] to the index of field[y][x]
void manage_leader(
	gamma_t *g, 
	uint32_t x,
	uint32_t y,
	uint32_t new_x,
	uint32_t new_y
) {
	uint64_t leader_index = find_leader(g, new_x, new_y);
	uint32_t leader_x = get_array_x_from_index(leader_index);
	uint32_t leader_y = get_array_y_from_index(leader_index);
	*get_arr_64(g->leader, leader_x, leader_y) = get_array_index(x, y);
}


bool gamma_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
	if (
		!check_gamma_exists(g) ||
		!check_player_correct(g, player) ||
		!check_field_correct(g, x, y) ||
		*get_arr_32(g->field, x, y) != 0 || (
			get_neighbour_count(g, player, x, y) == 0 &&
			g->players[player - 1]->occupied_areas == g->max_player_areas
		)
	) {
		return false;
	}

	// == sets [field]'s owner ==
	*get_arr_32(g->field, x, y) = player;
	g->players[player - 1]->taken_fields++;

	// == connects adjacent fields of the player into one area ==
	*get_arr_64(g->leader, x, y) = get_array_index(x, y);
	g->players[player - 1]->occupied_areas++;

	for (int i = 0; i < 4; i++) {
		uint32_t new_x = x + offset_x[i];
		uint32_t new_y = y + offset_y[i];
		if (
			check_field_exists(g, x, y, i) &&
			*get_arr_32(g->field, new_x, new_y) == player
		) {
			manage_leader(g, x, y, new_x, new_y);
		}
	}

	g->players[player - 1]->occupied_areas -=
		get_neighbour_count(g, player, x, y);

	// == manage all players available fields ==
	// managing the field that has been taken
	for (uint32_t i = 1; i <= g->player_count; i++) {
		if (get_neighbour_count(g, i, x, y) > 0) {
			g->players[i - 1]->available_fields_adjacent--;
		} else {
			g->players[i - 1]->available_fields_far--;
		}
	}

	// managing adjacent fields
	for (int i = 0; i < 4; i++) {
		uint32_t new_x = x + offset_x[i];
		uint32_t new_y = y + offset_y[i];
		if (
			check_field_exists(g, x, y, i) &&
			*get_arr_32(g->field, new_x, new_y) == 0 &&
			get_neighbour_count(g, player, new_x, new_y) == 1
		) {
			g->players[player - 1]->available_fields_adjacent++;
			g->players[player - 1]->available_fields_far--;
		}
	}

	return true;
}


// dfs that sets all connected player fields leader to [new_leader]
void set_leader(
	gamma_t *g,
	uint32_t player,
	uint64_t new_leader,
	uint32_t x,
	uint32_t y
) {
	*get_arr_64(g->leader, x, y) = new_leader;
	
	for (int i = 0; i < 4; i++) {
		uint32_t new_x = x + offset_x[i];
		uint32_t new_y = y + offset_y[i];
		if (
			check_field_exists(g, x, y, i) &&
			*get_arr_32(g->field, new_x, new_y) == player &&
			*get_arr_64(g->leader, new_x, new_y) != new_leader
		) {
			set_leader(g, player, new_leader, new_x, new_y);
		}
	}
}


// unlinks field[y][x] from [player]
void clear_field(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
	// == resets field owner ==
	*get_arr_32(g->field, x, y) = 0;
	g->players[player - 1]->taken_fields--;
	g->players[player - 1]->occupied_areas--;

	// == manage available_fields due to removing a players field ==
	// managing the field that has been cleared
	for (uint32_t i = 1; i <= g->player_count; i++) {
		if (get_neighbour_count(g, i, x, y) > 0) {
			g->players[i - 1]->available_fields_adjacent++;
		} else {
			g->players[i - 1]->available_fields_far++;
		}
	}

	// managing adjacent fields
	for (int i = 0; i < 4; i++) {
		uint32_t new_x = x + offset_x[i];
		uint32_t new_y = y + offset_y[i];
		if (
			check_field_exists(g, x, y, i) &&
			*get_arr_32(g->field, new_x, new_y) == 0 &&
			get_neighbour_count(g, player, new_x, new_y) == 0
		) {
			g->players[player - 1]->available_fields_adjacent--;
			g->players[player - 1]->available_fields_far++;
		}
	}

	// == fixes the leader of adjacent fields if they belong to [player] ==
	uint64_t last_leader = 0;

	for (int i = 0; i < 4; i++) {
		uint32_t new_x = x + offset_x[i];
		uint32_t new_y = y + offset_y[i];
		if (
			check_field_exists(g, x, y, i) &&
			*get_arr_32(g->field, new_x, new_y) == player &&
			*get_arr_64(g->leader, new_x, new_y) != last_leader
		) {
			set_leader(
				g,
				player,
				get_array_index(new_x, new_y),
				new_x,
				new_y
			);
			last_leader = *get_arr_64(g->leader, new_x, new_y);
			g->players[player - 1]->occupied_areas++;
		}
	}
}


bool gamma_golden_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
	if (
		!check_gamma_exists(g) ||
		!check_player_correct(g, player) ||
		g->players[player - 1]->used_golden_move ||
		!check_field_correct(g, x, y) ||
		*get_arr_32(g->field, x, y) == 0 ||
		*get_arr_32(g->field, x, y) == player
	) {
		return false;
	}

	// simulate the golden move
	uint32_t field_owner = *get_arr_32(g->field, x, y);
	g->players[player - 1]->used_golden_move = true;
	clear_field(g, field_owner, x, y);
	gamma_move(g, player, x, y);

	// check if the move was illegal (it created too many areas for any player),
	// if so, undo the move and return false
	for (uint32_t i = 0; i < g->player_count; i++) {
		if (g->players[i]->occupied_areas > g->max_player_areas) {
			g->players[player - 1]->used_golden_move = false;
			clear_field(g, player, x, y);
			gamma_move(g, field_owner, x, y);

			return false;
		}
	}

	// everything was fine
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

	uint64_t adjacent = g->players[player - 1]->available_fields_adjacent;
	uint64_t far = 0;
	if (g->players[player - 1]->occupied_areas < g->max_player_areas) {
		far = g->players[player - 1]->available_fields_far;
	}

	return adjacent + far;
}


bool gamma_golden_possible(gamma_t *g, uint32_t player) {
	if (!check_gamma_exists(g) || !check_player_correct(g, player)) {
		return false;
	}

	if (g->players[player - 1]->used_golden_move) {
		return false;
	}
	
	for (uint32_t i = 0; i < g->player_count; i++) {
		if (i != player - 1 && g->players[i]->taken_fields > 0) {
			return true;
		}
	}

	return false;
}


// self explanatory
uint64_t get_power_of_ten(uint32_t number) {
	uint64_t res = 1;
	
	while (number /= 10) {
		res++;
	}

	return res;
}


// adds digits of [number] to [string] at positions
// ([size], [size] + get_power_of_ten([number] - 1))
void add_number_to_string(char **string, uint32_t number, uint64_t *size) {
	uint64_t position = *size + get_power_of_ten(number) - 1;

	while (number > 0) {
		*string[position--] = (number % 10) + '0';
		number /= 10;
		(*size)++;
	}
}


char* gamma_board(gamma_t *g) {
	uint64_t bonus_brackets_space = 0;
	for (uint32_t i = 10; i < g->player_count; i++) {
		bonus_brackets_space += 
			get_power_of_ten(g->players[i]->taken_fields) + 2;
	}
	uint64_t space_required = 
		(g->field_width + 1) * g->field_height + bonus_brackets_space + 1;
	
	char *gamma_to_string = malloc(space_required * sizeof(char));
	if (!gamma_to_string) {
		return NULL;
	}

	uint64_t size = 0;
	long long safe_stop = g->field_height - 1;
	for (uint32_t y = g->field_height - 1; safe_stop >= 0; y--, safe_stop--) {
		for (uint32_t x = 0; x < g->field_width; x++) {
			if (*get_arr_32(g->field, x, y) == 0) {
				gamma_to_string[size++] = '.';
			}
			else if (*get_arr_32(g->field, x, y) < 10) {
				gamma_to_string[size++] = *get_arr_32(g->field, x, y) + '0';
			}
			else {
				gamma_to_string[size++] = '[';
				add_number_to_string(
					&gamma_to_string,
					*get_arr_32(g->field, x, y),
					&size
				);
				gamma_to_string[size++] = ']';
			}
		}
		gamma_to_string[size++] = '\n';
	}
	gamma_to_string[size++] = 0;

	return gamma_to_string;
}
