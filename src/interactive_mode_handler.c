/** @file
 * Implementacja modulu obslugujacego tryb interaktywny gry gamma
 * 
 * @author Piotr Prabucki <pp418377@students.mimuw.edu.pl>
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
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
 * Podswietla komorke ( @p cursor_pos_x, @p cursor_pos_y ).
 * Podswietla rowniez na inny kolor pozostale pola zajete przez gracza
 * @p player.
 * 
 * @param[in] gamma         : wskaznik na strukture przechowujaca informacje o
 *                            grze
 * @param[in] cell_size     : rozmiar komorki planszy na wyjsciu
 * @param[in] player        : numer gracza, ktorego tura jest rozgrywana
 * @param[in] cursor_pos_x  : numer kolumny komorki, na ktorej jest kursor
 * @param[in] cursor_pos_y  : numer wiersza komorki, na ktorej jest kursor
 */
void print_board(
	gamma_t *gamma, 
	uint32_t cell_size,
	uint32_t player,
	long long cursor_pos_x,
	long long cursor_pos_y
) {
	for (long long y = gamma->field_height - 1; y >= 0; y--) {
		for (uint32_t x = 0; x < gamma->field_width; x++) {
			// highlighting the cursor position
			if (y == gamma->field_height - cursor_pos_y && x == cursor_pos_x) {
				printf("\033[46;1m");
			}
			else if (*get_arr_32(gamma->field, x, y) == player) {
				printf("\033[44;1m");
			}

			// printing the cell's value or '.' if its not occupied
			if (*get_arr_32(gamma->field, x, y) > 0) {
				printf("%*u", cell_size, *get_arr_32(gamma->field, x, y));
			} else {
				char *dot = ".";
				printf("%*s", cell_size, dot);
			}

			// ending the highlighting for the cursor
			if (
				(y == gamma->field_height - cursor_pos_y &&
					x == cursor_pos_x) ||
				*get_arr_32(gamma->field, x, y) == player
			) {
				printf("\033[0m");
			}

			// adding a space if its not the last cell in the row
			if (x != gamma->field_width - 1) {
				printf(" ");
			}
		}
		printf("\n");
	}
	
	printf(
		"PLAYER: %d\nTAKEN FIELDS: %lu\nAVAILABLE FIELDS: %lu\n", 
		player,
		gamma_busy_fields(gamma, player),
		gamma_free_fields(gamma, player)
	);
	printf("GOLDEN MOVE: ");
	if (gamma->players[player - 1]->used_golden_move) {
		printf("UN");
	}
	printf("AVAILABLE\n");
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

	print_board(gamma, cell_size - 1, player, row, column);

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
	if (
		boundary_down <= *cursos_pos + diff && 
		*cursos_pos + diff < boundary_up
	) {
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
void handle_interactive_input(
	gamma_t **gamma,
	long long boundary_x,
	long long boundary_y,
	uint32_t max_player_number
) {
	int cell_size = get_cell_size(max_player_number) + 1;
	long long cursor_pos_x = 0, cursor_pos_y = 1;
	long long player = 1;

	reset_screen(*gamma, cursor_pos_x, cursor_pos_y, cell_size, player);

	char input;
	int flag_escape = 0, flag_escape_followed = 0;
	while (!feof(stdin)) {
		input = getc(stdin);

		switch (input) {
			case 4: // CTRL - D
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


/**
 * @brief Sprawdza, czy terminal jest wystarczajaco duzy, by przeprowadzic
 * gre @p gamma.
 * 
 * @param[in] gamma : wskaznik na strukture przechowujaca informacje o grze
 * 
 * @return Zwraca 1, gdy gra moze zostac przeprowadzona oraz 0 w przeciwnym
 * wypadku.
 */
int validate_terminal_size(gamma_t *gamma) {
	struct winsize info;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &info);

	int cell_size = get_cell_size(gamma->player_count) + 1;

	int game_width = cell_size * gamma->field_width - 1;
	int game_height = gamma->field_height + 4;

	return info.ws_col > game_width && info.ws_row > game_height;
}



/**
 * @brief Odpala tryb interaktywny gry @p gamma.
 * Zmienia ustawienia wyswietlania terminala, by odpowiednio wypisywac stan gry.
 * Przywraca te ustawienia przy wyjsciu z funkcji.
 * Jesli rozmiar terminala jest za maly, by przeprowadzic zadana rozgrywke,
 * konczy program z kodem 1.
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
	if (!validate_terminal_size(*gamma)) {
		printf("Rozmiar terminala jest za maly, by przeprowadzic rozgrywke\n");
		exit(1);
	}

	struct termios new_termios;
	struct termios old_termios;

	int operation_failed = tcgetattr(0, &old_termios);
	if (operation_failed) {
		exit(1);
	}

	memcpy(&new_termios, &old_termios, sizeof(struct termios));

	new_termios.c_lflag &= ~(ICANON | ECHO);
	new_termios.c_cc[VTIME] = 0;
	new_termios.c_cc[VMIN] = 1;

	operation_failed = tcsetattr(0, TCSANOW, &new_termios);
	if (operation_failed) {
		exit(1);
	}

	printf("\033[?25l");
	handle_interactive_input(gamma, boundary_x, boundary_y, max_player_number);
	printf("\033[?25h");

	clear_screen();
	operation_failed = tcsetattr(0, TCSANOW, &old_termios);
	if (operation_failed) {
		exit(1);
	}

	return;
}
