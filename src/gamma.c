#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "memory_util.h"


typedef struct player {
	bool used_golden_move;
	uint64_t taken_fields;
	uint64_t available_fields;
} player_t;


player_t* player_new(uint64_t height, uint64_t width) {
	player_t *player = malloc(sizeof(player_t));
	if (!player) {
		return NULL;
	}

	player->used_golden_move = false;
	player->taken_fields = 0;
	player->available_fields = height * width;

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
	if (!g) {
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


bool gamma_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
	// do stuff
}


bool gamma_golden_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
	// do stuff
}


uint64_t gamma_busy_fields(gamma_t *g, uint32_t player) {
	if (!g || player == 0 || player > g->player_count) {
		return 0;
	}

	return g->players[player - 1]->taken_fields;
}


uint64_t gamma_free_fields(gamma_t *g, uint32_t player) {
	if (!g || player == 0 || player > g->player_count) {
		return 0;
	}

	return g->players[player]->available_fields;
}


bool gamma_golden_possible(gamma_t *g, uint32_t player) {
	if (!g || player == 0 || player > g->player_count) {
		return false;
	}

	bool res = !g->players[player - 1]->used_golden_move;
	if (!res) {
		return res;
	}
	
	for (int i = 0; i < g->player_count; i++) {
		if (i != player - 1) {
			res = g->players[i]->taken_fields > 0;
		}
	}

	return res;
}


char* gamma_board(gamma_t *g) {
	// do stuff
}
