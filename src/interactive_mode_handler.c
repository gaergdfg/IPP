/** @file
 * Implementacja modulu obslugujacego tryb interaktywny gry gamma
 * 
 * @author Piotr Prabucki <pp418377@students.mimuw.edu.pl>
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include "gamma.h"
#include "array_util.h"


/**
 * @brief Oblicza rozmiar komorki planszy sluzacy do wypisywania planszy gry w
 * @ref print_board.
 * 
 * @param[in] number    : najwiekszy numer gracza
 * 
 * @return Rozmiar komorki planszy.
 */
int get_cell_size(uint32_t number) {
	int res = 1;

	while (number /= 10) {
		res++;
	}

	return res;
}


/**
 * @brief Wyswietla plansze gry.
 * Wypisuje stan gry na standardowe wyjscie.
 * 
 * @param[in] gamma     : wskaznik na strukture przechowujaca informacje o grze
 * @param[in] cell_size : rozmiar komorki planszy na wyjsciu
 * @param[in] player    : numer gracza, ktorego tura jest rozgrywana
 */
void print_board(gamma_t *gamma, uint32_t cell_size, uint32_t player) {
	for (long long y = gamma->field_height - 1; y >= 0; y--) {
		for (uint32_t x = 0; x < gamma->field_width; x++) {
			if (x != gamma->field_width - 1) {
				printf("%*d ", cell_size, *get_arr_32(gamma->field, x, y));
			}
			else {
				printf("%*d", cell_size, *get_arr_32(gamma->field, x, y));
			}
		}
		printf("\n");
	}
	printf("Tura gracza #%d\n", player);
	printf("Zloty ruch ");
	if (gamma->players[player - 1]->used_golden_move) {
		printf("nie");
	}
	printf("dostepny\n");
}


/**
 * @brief Czysci wyjscie, ustawia kursor na (0, 0).
 */
void clear_screen() {
	printf("\033[2J");
	printf("\033[0;0H");
}


/**
 * @brief Odswieza wyswietlany stan gry.
 * 
 * @param[in] gamma     : wskaznik na strukture przechowujaca stan gry
 * @param[in] row       : numer wiersza, na ktorym znajduje sie kursor
 * @param[in] column    : numer kolumny, na ktorej znajduje sie kursor
 * @param[in] cell_size : rozmiar komorki planszy na wyjsciu
 * @param[in] player    : numer gracza, ktorego tura jest rozgrywana
 */
void reset_screen(
	gamma_t *gamma,
	int row,
	int column,
	int cell_size,
	uint32_t player
) {
	clear_screen();

	print_board(gamma, cell_size - 1, player);

	printf("\033[%d;%dH", column, (row + 1) * cell_size - 1);
}


/**
 * @brief Przesuwa kursor o @p diff jesli jest to mozliwe.
 * Zmienia wartosc @p cursor_pos, jesli @p cursor_pos + @p diff zawiera sie
 * miedzy @p boundary_down a @p boundary_down - 1 (oba wlacznie).
 * 
 * @param[in, out] cursor_pos   : wskaznik na zmienna trzymajaca pozycje kursora
 * @param[in] diff              : wartosc, o ktora przesuwamy @p cursor_pos
 * @param[in] boundary_down     : minimalna poprawna pozycja kursora
 * @param[in] boundary_up       : maksymalna poprawna pozycja kursora
 */
void move_cursor(
	long long *cursos_pos,
	long long diff,
	long long boundary_down,
	long long boundary_up
) {
	if (boundary_down <= *cursos_pos + diff && *cursos_pos + diff < boundary_up) {
		*cursos_pos += diff;
	}
}


/**
 * @brief Znajduje numer nastepnego gracza, ktory jest w stanie wykonac ruch w
 * grze @p gamma.
 * 
 * @param[in] gamma             : wskaznik na strukture przechowujaca stan gry
 * @param[in] player            : numer gracza, ktorego tura sie skonczyla
 * @param[in] max_player_number : liczba graczy w grze @p gamma
 * 
 * @return Numer gracza kolejnego gracza, jesli jakikolwiek z nich moze jeszcze
 * wykonac ruch lub -1 w przeciwnym wypadku.
 */
long long get_next_player(
	gamma_t *gamma,
	uint32_t player,
	uint32_t max_player_number
) {
	long long next_player = player + 1;

	while (next_player != player) {
		if (next_player > max_player_number) {
			next_player = 1;
		}
		if (
			gamma_free_fields(gamma, next_player) > 0 ||
			gamma_golden_possible(gamma, next_player)
		) {
			return next_player;
		}
		next_player++;
	}

	return -1;
}


/**
 * @brief Obsluguje tryb interaktywny gry gamma dla danej planszy @p gamma.
 * Zmienia stan gry @p gamma zgodnie z poleceniami przekazywanymi programowi.
 * 
 * @param[in, out] gamma        : wskaznik na wskaznik na strukture
 *                                przechowujaca stan gry
 * @param[in] boundary_x        : maksymalna poprawna pozycja kursora na osi X
 * @param[in] boundary_y        : maksymalna poprawna pozycja kursora na osi Y
 * @param[in] max_player_number : liczba graczy w grze @p gamma
 */
void run_interactive_mode(
	gamma_t **gamma,
	long long boundary_x,
	long long boundary_y,
	uint32_t max_player_number
) {
	struct termios new_kbd_mode;
	struct termios g_old_kbd_mode;

	bool operation_failed = tcgetattr(0, &g_old_kbd_mode);
	if (operation_failed) {
		exit(1);
	}

	memcpy(&new_kbd_mode, &g_old_kbd_mode, sizeof (struct termios));

	new_kbd_mode.c_lflag &= ~(ICANON | ECHO);
	new_kbd_mode.c_cc[VTIME] = 0;
	new_kbd_mode.c_cc[VMIN] = 1;

	operation_failed = tcsetattr(0, TCSANOW, &new_kbd_mode);
	if (operation_failed) {
		exit(1);
	}

	int cell_size = get_cell_size(max_player_number) + 1;
	long long cursor_pos_x = 0, cursor_pos_y = 1;
	long long player = 1;

	reset_screen(*gamma, cursor_pos_x, cursor_pos_y, cell_size, player);

	char input;
	int flag_escape = 0, flag_escape_followed = 0;
	while ((input = getc(stdin))) {
		switch (input) {
			case 4: // CTRL - D
				clear_screen();
				tcsetattr(0, TCSANOW, &g_old_kbd_mode);
				return;
			case 27: // ESCAPE
				flag_escape = 1;
				break;
			case 91: // [
				if (flag_escape) {
					flag_escape_followed = 1;
				}
				break;
			case 65: // UP
				if (flag_escape_followed) {
					move_cursor(&cursor_pos_y, -1, 1, boundary_y);
				}
				break;
			case 'c':
			case 67: // RIGHT | 'C'
				if (flag_escape_followed) {
					move_cursor(&cursor_pos_x, 1, 0, boundary_x);
				}
				else {
					player = get_next_player(*gamma, player, max_player_number);
					if (player == -1) {
						clear_screen();
						tcsetattr(0, TCSANOW, &g_old_kbd_mode);
						return;
					}
				}
				break;
			case 66: // DOWN
				if (flag_escape_followed) {
					move_cursor(&cursor_pos_y, 1, 1, boundary_y);
				}
				break;
			case 68: // LEFT
				if (flag_escape_followed) {
					move_cursor(&cursor_pos_x, -1, 0, boundary_x);
				}
				break;
			case 32: // SPACE
				if (
					gamma_move(
						*gamma, 
						player,
						cursor_pos_x,
						boundary_y - cursor_pos_y - 1
					)
				) {
					player = get_next_player(*gamma, player, max_player_number);
					if (player == -1) {
						clear_screen();
						tcsetattr(0, TCSANOW, &g_old_kbd_mode);
						return;
					}
				}
				break;
			case 'g':
			case 'G':
				if (
					gamma_golden_move(
						*gamma,
						player,
						cursor_pos_x,
						boundary_y - cursor_pos_y - 1
					)
				) {
					player = get_next_player(*gamma, player, max_player_number);
					if (player == -1) {
						clear_screen();
						tcsetattr(0, TCSANOW, &g_old_kbd_mode);
						return;
					}
				}
				break;
			default:
				break;
		}

		if (flag_escape && input != 27) {
			flag_escape = 0;
		}
		if (flag_escape_followed && input != 91) {
			flag_escape_followed = 0;
		}

		reset_screen(*gamma, cursor_pos_x, cursor_pos_y, cell_size, player);
	}
}
