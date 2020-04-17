/** @file
 * Implementacja silnika gry gamma
 *
 * @author Piotr Prabucki <pp418377@students.mimuw.edu.pl>
 */


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "array_util.h"
#include "memory_util.h"


/**
 * Tablica uzywana do iterowania sie po sasiadujacych komorkach na polu gry na
 * osi x. Kolejnosc: gora, prawo, dol, lewo.
 */
const uint32_t offset_x[4] = {0, 1, 0, -1};
/**
 * Tablica uzywana do iterowania sie po sasiadujacych komorkach na polu gry na
 * osi y. Kolejnosc: gora, prawo, dol, lewo.
 */
const uint32_t offset_y[4] = {-1, 0, 1, 0};



/**
 * Struktura przechowujaca dane gracza.
 * 
 * @param used_golden_move          : wartosc logiczna mowiaca, czy gracz
 *                                    uzyl juz zloty ruch
 * @param taken_fields              : liczba pol zajetych przez gracza
 * @param available_fields_adjacent : liczba pol mozliwych do zajecia przez
 *                                    gracza bez koniecznosci wykorzystania
 *                                    jeszcze jednego obszaru
 * @param available_fields_far      : liczba pol mozliwych do zajecia przez
 *                                    gracza z koniecznoscia wykorzystania
 *                                    jeszcze jednego obszaru
 * @param occupied_areas            : liczba obszarow zajetych przez gracza
 */
typedef struct player {
	bool used_golden_move;
	uint64_t taken_fields;
	uint64_t available_fields_adjacent;
	uint64_t available_fields_far;
	uint32_t occupied_areas;
} player_t;


/**
 * @brief Tworzy strukture przechowujaca dane gracza.
 * Alokuje pamiec na nowa strukture.
 * Inicjalizuje te strukture tak, aby reprezentowala poczatkowe dane gracza.
 * 
 * @param[in] height    : wysokosc planszy, na ktorej gracz bedzie gral
 * @param[in] width     : szerokosc planszy, na ktorej gracz bedzie gral
 * 
 * @return Wskaznik na utworzona strukture lub NULL, gdy nie udalo sie
 * zaalokowac pamieci.
 */
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


/**
 * Struktura przechowujaca stan gry.
 * 
 * @param field_height      : wysokosc planszy
 * @param field_width       : szerokosc planszy
 * @param player_count      : maksymalna liczba graczy grajacych w te gre
 * @param max_player_areas  : maksymalna liczba obszarow, jakie moze posiadac
 *                            gracz w danym momencie gry
 * @param field             : reprezentacja planszy gry (tablica dwuwymiarowa),
 *                            ktora w danym miejscu trzyma numer gracza, ktorego
 *                            pionek stoi na tym polu lub 0, gdy zaden gracz nie
 *                            ma pionka na tym polu
 * @param leader            : tablica dwuwymiarowa zawierajaca lidera z
 *                            algorytmu Find & Union uzywanego do zliczania
 *                            obszarow zajetych przez gracza
 * 
 * @param max_player_areas  : maksymalna liczba obszarow, ktore gracz moze
 *                            zajmowac w danym momencie gry
 */
typedef struct gamma {
	uint32_t field_height;
	uint32_t field_width;
	uint32_t player_count;
	uint32_t max_player_areas;

	uint32_t** field;
	uint64_t** leader;
	player_t** players;
} gamma_t;


/**
 * @brief Sprawdza, czy gracz o danym numerze istnieje w grze.
 * Sprawdza, czy numer [player] jest mniejszy lub rowny maksymalnej liczby
 * graczy w grze [g] oraz, czy jest wiekszy od zera.
 * 
 * @param[in,out] g     : wskaznik na strukture przechowujaca stan gry
 * @param[in] player	: numer gracza, ktory chcemy sprawdzic
 * 
 * @return true, gdy numer gracza nie przekracza maksymalnej liczby graczy
 * w grze lub false w przeciwnym przypadku
 */
bool check_player_correct(gamma_t* g, uint32_t player) {
	return player > 0 && player <= g->player_count;
}


/**
 * @brief Sprawdza, czy pole o wspolrzednych [x], [y] jest poprawne.
 * Sprawdza, czy [x] i [y] nie przekraczaja wymiarow planszy w grze [g].
 * 
 * @param[in,out] g : wskaznik na strukture przechowujaca stan gry
 * @param[in] x     : wspolrzedna osi X pola
 * @param[in] y     : wspolrzedna osi Y pola
 * 
 * @return true, gdy wspolrzedne nie przekraczaja wymiarow planszy w grze g,
 * false w przeciwnym wypadku
 */
bool check_field_correct(gamma_t* g, uint32_t x, uint32_t y) {
	return x < g->field_width && y < g->field_height;
}


/**
 * @brief Sprawdza, czy mozemy sie poruszyc na poly gry [g] w kierunku [dir] ze
 * wspolrzednych [x], [y].
 * Kierunek [dir] oznacza:
 * 		0 - gora,
 * 		1 - prawo,
 * 		2 - dol,
 * 		3 - lewo.
 * Gdy [dir] > 3, rozpatruje przypadek dir = 3.
 * 
 * @param[in,out] g     : wskaznik na strukture przechowujaca stan gry
 * @param[in] x         : wspolrzedna osi X 
 * @param[in] y         : wspolrzedna osi Y
 * @param[in] dir       : liczba reprezentujaca kierunek, w ktorym chcemy sie
 *                    poruszyc
 * 
 * @return true, gdy mozemy sie poruszyc w danym kierunku, false w przeciwnym
 * wypadku
 */
bool check_field_exists(gamma_t *g, uint32_t x, uint32_t y, uint32_t dir) {
	if (dir == 0) {
		return y > 0; // up, [y - 1][x]
	}
	if (dir == 1) {
		return x + 1 < g->field_width; // right, [y][x + 1]
	}
	if (dir == 2) {
		return y + 1 < g->field_height; // down, [y + 1][x]
	}
	return x > 0; // left, [y][x - 1]
}


/**
 * Zwraca liczbe pol sasiadujacych do pola o wspolrzednych [x], [y] oraz
 * nalezacych do gracza [player].
 * 
 * @param[in,out] g     : wskaznik na strukture przechowujaca stan gry
 * @param[in] player	: gracz, ktorego pola sasiadujace do pola o
 *                        wspolrzednych [x], [y] liczymy
 * @param[in] x         : wspolrzedna osi X
 * @param[in] y         : wspolrzedna osi Y
 * 
 * @return Liczba pol sasiadujacych do pola o wspolrzednych [x], [y] oraz
 * nalezacych do gracza [player].
 */
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


/**
 * @brief Znajduje lidera pola o wspolrzednych [x], [y].
 * Znajduje lidera pola zgodnie z algorytmem Find & Union, przestawiajac
 * liderow pol, przez ktore przechodzi, by przyspieszyc kolejne wywolania
 * funkcji.
 * 
 * @param[in,out] g : wskaznik na strukture przechowujaca stan gry
 * @param[in] x     : wspolrzedna osi X pola
 * @param[in] y     : wspolrzedna osi Y pola
 * 
 * @return Lider pola.
 */
uint64_t find_leader(gamma_t *g, uint32_t x, uint32_t y) {
	uint64_t curr_leader = *get_arr_64(g->leader, x, y);
	uint64_t array_index = get_array_index(x, y);
	if (curr_leader == array_index) {
		return curr_leader;
	}
	if (curr_leader == 0) {
		return get_array_index(x, y);
	}

	uint32_t leader_x = get_array_x_from_index(curr_leader);
	uint32_t leader_y = get_array_y_from_index(curr_leader);
	uint64_t new_leader = find_leader(g, leader_x, leader_y);
	*get_arr_64(g->leader, x, y) = new_leader;

	return new_leader;
}


/** @brief Tworzy strukturę przechowującą stan gry.
 * Alokuje pamięć na nową strukturę przechowującą stan gry.
 * Inicjuje tę strukturę tak, aby reprezentowała początkowy stan gry.
 * 
 * @param[in] width   : szerokość planszy, liczba dodatnia,
 * @param[in] height  : wysokość planszy, liczba dodatnia,
 * @param[in] players : liczba graczy, liczba dodatnia,
 * @param[in] areas   : maksymalna liczba obszarów,
 *                      jakie może zająć jeden gracz.
 * 
 * @return Wskaźnik na utworzoną strukturę lub NULL, gdy nie udało się
 * zaalokować pamięci lub któryś z parametrów jest niepoprawny.
 */
gamma_t* gamma_new(
	uint32_t width,
	uint32_t height,
	uint32_t players,
	uint32_t areas
) {
	if (width == 0 || height == 0 || players == 0 || areas == 0) {
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

	g->field = allocate_2d_array_uint32(height, width);
	if (!g->field) {
		free(g);
		return NULL;
	}

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


/** @brief Usuwa strukturę przechowującą stan gry.
 * Usuwa z pamięci strukturę wskazywaną przez @p g.
 * Nic nie robi, jeśli wskaźnik ten ma wartość NULL.
 * 
 * @param[in,out] g : wskaźnik na usuwaną strukturę.
 */
void gamma_delete(gamma_t *g) {
	if (!g) {
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


/**
 * @brief Ustawia lidera pola o wspolrzednych [new_x], [new_y] na indeks pola o
 * wspolrzednych [x], [y].
 * Ustawia lidera pola [new_y][new_x] na indeks pola [y][x] (z funkcji @ref
 * get_array_index) zgodnie z algorytmem Find & Union.
 * 
 * @param[in,out] g : wskaznik na strukture przechowujaca stan gry
 * @param[in] x     : wspolrzedna osi X pola, ktorego ustawiamy jako lidera
 * @param[in] y     : wspolrzedna osi Y pola, ktorego ustawiamy jako lidera
 * @param[in] new_x : wspolrzedna osi X pola, ktorego lidera zmieniamy
 * @param[in] new_y : wspolrzedna osi Y pola, ktorego lidera zmieniamy
 */
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


/** @brief Wykonuje ruch.
 * Ustawia pionek gracza @p player na polu (@p x, @p y).
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli ruch został wykonany, a @p false,
 * gdy ruch jest nielegalny lub któryś z parametrów jest niepoprawny.
 */
bool gamma_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
	if (
		!g ||
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

	uint64_t previous_leaders[4] = {0, 0, 0, 0};
	for (int i = 0; i < 4; i++) {
		uint32_t new_x = x + offset_x[i];
		uint32_t new_y = y + offset_y[i];
		if (
			check_field_exists(g, x, y, i) &&
			*get_arr_32(g->field, new_x, new_y) == player
		) {
			bool is_different = true;
			uint64_t leader = find_leader(g, new_x, new_y);
			for (long long j = i - 1; j >= 0 && is_different; j--) {
				if (leader == previous_leaders[j]) {
					is_different = false;
				}
			}
			if (is_different) {
				g->players[player - 1]->occupied_areas--;
			}
			previous_leaders[i] = leader;
		}
	}

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

	// == manage all players available fields ==
	// managing the field that has been taken
	for (uint32_t i = 1; i <= g->player_count; i++) {
		if (get_neighbour_count(g, i, x, y) > 0) {
			g->players[i - 1]->available_fields_adjacent--;
		}
		else {
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


/**
 * @brief Ustawia lidera wszystkich pol nalezacych do tego samego obszary, co
 * pole o wspolrzednych [x], [y] na [new_leader].
 * Uzywa algorytmu DFS.
 * 
 * @param[in,out] g         : wskaznik na strukture przechowujaca stan gry
 * @param[in] player        : numer gracza, ktorego liderow obszaru zmieniamy
 * @param[in] new_leader	: nowy lider, ktorego ustawiamy na wszystikch
 *                            polach
 * @param[in] x             : wspolrzedna osi X pola, na ktorym obecnie
 *                            jestesmy
 * @param[in] y             : wspolrzedna osi Y pola, na ktorym obecnie
 *                            jestesmy
 */
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


/**
 * @brief Zabiera pole o wspolrzednych [x], [y] od gracza o numerze [player].
 * Ustawia pole [y][x] jako pole niczyje, nastepnie aktualizuje liderow
 * sasiednich pol nalezacych do gracza [player] oraz aktualizuje pola dostepne
 * wszystkich graczy.
 * 
 * @param[in,out] g     : wskaznik na strukture przechowujaca stan gry
 * @param[in] player    : numer gracza, od ktorego zabieramy pole
 * @param[in] x         : wspolrzednia osi X pola, ktore zabieramy graczowi
 * @param[in] y         : wspolrzednia osi Y pola, ktore zabieramy graczowi
 */
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
		}
		else {
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

	set_leader(g, player, 0, x, y);
	for (int i = 0; i < 4; i++) {
		uint32_t new_x = x + offset_x[i];
		uint32_t new_y = y + offset_y[i];
		if (
			check_field_exists(g, x, y, i) &&
			*get_arr_32(g->field, new_x, new_y) == player &&
			find_leader(g, new_x, new_y) != last_leader
		) {
			set_leader(
				g,
				player,
				get_array_index(new_x, new_y),
				new_x,
				new_y
			);
			last_leader = get_array_index(new_x, new_y);
			g->players[player - 1]->occupied_areas++;
		}
	}
}


/** @brief Wykonuje zloty ruch.
 * Ustawia pionek gracza @p player na polu (@p x, @p y) zajetym przez innego
 * gracza, usuwajac pionek innego gracza. Nastepnie sprawdza, czy ten ruch
 * nie doprowadzil do sytuacji, w ktorej pewien gracz ma za duzo zajetych
 * obszarow a jesli tak sie stalo, przywraca plansze do stanu przed wywolaniem
 * funkcji.
 * 
 * @param[in,out] g   : wskaznik na strukturę przechowujaca stan gry,
 * @param[in] player  : numer gracza, liczba dodatnia niewieksza od wartosci
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       : numer kolumny, liczba nieujemna mniejsza od wartosci
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       : numer wiersza, liczba nieujemna mniejsza od wartosci
 *                      @p height z funkcji @ref gamma_new.
 * 
 * @return Wartosć @p true, jesli ruch zostal wykonany, a @p false,
 * gdy gracz wykorzystal już swoj zloty ruch, ruch jest nielegalny
 * lub ktorys z parametrow jest niepoprawny.
 */
bool gamma_golden_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
	if (
		!g ||
		!check_player_correct(g, player) ||
		g->players[player - 1]->used_golden_move ||
		!check_field_correct(g, x, y) ||
		*get_arr_32(g->field, x, y) == 0 ||
		*get_arr_32(g->field, x, y) == player
	) {
		return false;
	}

	bool undo_golden_move = false;
	bool move_failed = false;

	// simulate the golden move
	uint32_t field_owner = *get_arr_32(g->field, x, y);
	g->players[player - 1]->used_golden_move = true;
	clear_field(g, field_owner, x, y);

	if (!gamma_move(g, player, x, y)) {
		undo_golden_move = true;
		move_failed = true;
	}

	// check if the move was illegal (it created too many areas for any player),
	// if so, undo the move and return false
	for (uint32_t i = 0; i < g->player_count && !undo_golden_move; i++) {
		if (g->players[i]->occupied_areas > g->max_player_areas) {
			undo_golden_move = true;
		}
	}

	if (undo_golden_move && !move_failed) {
		clear_field(g, player, x, y);
	}
	if (undo_golden_move || move_failed) {
		g->players[player - 1]->used_golden_move = false;
		gamma_move(g, field_owner, x, y);
	}

	return !undo_golden_move;
}


/** @brief Podaje liczbę pól zajętych przez gracza.
 * Podaje liczbę pól zajętych przez gracza @p player.
 * 
 * @param[in,out] g   : wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  : numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * 
 * @return Liczba pól zajętych przez gracza lub zero,
 * jeśli któryś z parametrów jest niepoprawny.
 */
uint64_t gamma_busy_fields(gamma_t *g, uint32_t player) {
	if (!g || !check_player_correct(g, player)) {
		return 0;
	}

	return g->players[player - 1]->taken_fields;
}


/** @brief Podaje liczbę pól, jakie jeszcze gracz może zająć.
 * Podaje liczbę wolnych pól, na których w danym stanie gry gracz @p player może
 * postawić swój pionek w następnym ruchu.
 * 
 * @param[in,out] g   : wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  : numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * 
 * @return Liczba pól, jakie jeszcze może zająć gracz lub zero,
 * jeśli któryś z parametrów jest niepoprawny.
 */
uint64_t gamma_free_fields(gamma_t *g, uint32_t player) {
	if (!g || !check_player_correct(g, player)) {
		return 0;
	}

	uint64_t adjacent = g->players[player - 1]->available_fields_adjacent;
	uint64_t far = 0;
	if (g->players[player - 1]->occupied_areas < g->max_player_areas) {
		far = g->players[player - 1]->available_fields_far;
	}

	return adjacent + far;
}


/** @brief Sprawdza, czy gracz może wykonać złoty ruch.
 * Sprawdza, czy gracz @p player jeszcze nie wykonał w tej rozgrywce złotego
 * ruchu i jest przynajmniej jedno pole zajęte przez innego gracza.
 * 
 * @param[in,out] g   : wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  : numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * 
 * @return Wartość @p true, jeśli gracz jeszcze nie wykonał w tej rozgrywce
 * złotego ruchu i jest przynajmniej jedno pole zajęte przez innego gracza,
 * a @p false w przeciwnym przypadku.
 */
bool gamma_golden_possible(gamma_t *g, uint32_t player) {
	if (!g || !check_player_correct(g, player)) {
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


/** 
 * Zwraca potege, do ktorej zostala podniesiona 10 w najwiekszej potedze 10,
 * mniejszej od liczby [number]
 * 
 * @param[in] number    : liczba, o ktora pytamy
 * 
 * @return Potega, do ktorej zostala podniesiona 10 w najwiekszej potedze 10,
 * mniejszej od liczby [number].
 */
uint64_t get_power_of_ten(uint32_t number) {
	uint64_t res = 1;
	
	while (number /= 10) {
		res++;
	}

	return res;
}


/**
 * Dodaje cyfry liczby [number] do napisu [string] na pozycjach
 * ([size], [size] + @ref get_power_of_ten([number] - 1)).
 * 
 * @param[in] string    : napis, do ktorego dodajemy cyfry
 * @param[in] number    : liczba, ktorej cyfry dodajemy do napisu
 * @param[in] size      : pozycja, na ktorej zaczynamy dodawanie cyfr do napisu
 */
void add_number_to_string(char **string, uint32_t number, uint64_t *size) {
	uint64_t position = *size + get_power_of_ten(number) - 1;

	while (number > 0) {
		*string[position--] = (number % 10) + '0';
		number /= 10;
		(*size)++;
	}
}


/** @brief Daje napis opisujący stan planszy.
 * Alokuje w pamięci bufor, w którym umieszcza napis zawierający tekstowy
 * opis aktualnego stanu planszy. Przykład znajduje się w pliku gamma_test.c.
 * Funkcja wywołująca musi zwolnić ten bufor.
 * 
 * @param[in] g       : wskaźnik na strukturę przechowującą stan gry.
 * 
 * @return Wskaźnik na zaalokowany bufor zawierający napis opisujący stan
 * planszy lub NULL, jeśli nie udało się zaalokować pamięci.
 */
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
