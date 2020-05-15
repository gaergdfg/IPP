/** @file
 * Implementacja modulu analizujacego skladnie wejscia
 * 
 * @author Piotr Prabucki <pp418377@students.mimuw.edu.pl>
 */


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"


/**
 * @brief Sprawdza, czy @p c jest dozwolonym bialym znakiem.
 * 
 * @param[in] c : znak, ktory sprawdzamy
 * 
 * @return Jesli @p c jest dozwolonym bialym znakiem zwraca 1, w przeciwnym
 * wypadku 0.
 */
int check_white_character(char c) {
	return c == 9 || c == 11 || c == 12 || c == 13 || c == 32;
}


/**
 * @brief Sprawdza, czy @p instruction zawiera niedozwolone biale znaki.
 * 
 * @param[in] instruction   : wskaznik na poczatek napisu
 * @param[in] size          : dlugosc napisu
 * 
 * @return Gdy w napisie sa niedozwolone biale znaki zwraca 1, w przeciwnym
 * wypadku 0.
 */
int check_for_illegal_signs(char *instruction, int size) {
	unsigned char c;
	for (int i = 0; i < size; i++) {
		c = instruction[i];
		if (c < 32 && c != 9 && c != 11 && c != 12 && c != 13) {
			return 1;
		}
	}
	return 0;
}


/**
 * @brief Czyta linie ze standardowego wejscia.
 * Wczytuje linie (zatrzymuje sie na '\n' lub EOF).
 * Dynamicznie alokuje pamiec, tak by starczylo jej do wczytania wejscia.
 * Ustawia wskaznik @p line tam, gdzie zostala zaalokowana pamiec.
 * 
 * @param[in,out] line      : wskaznik na wczytywana linie
 * @param[in] buffer_size   : wstepny rozmiar bufora
 * 
 * @return Zwraca dlugosc linii, gdy wejscie mialo poprawny format.
 * Zwracane wartosci w przeciwnym wypadku:
 * 0 (pusta linia konczaca sie '\n')
 * -1 (pierwszy wczytany znak to EOF)
 * -2 (niepusta linia konczaca sie EOF)
 */
int read_line(char **line, int buffer_size) {
	int size = 0;

	while (1) {
		char input = 0;

		for (;size < buffer_size - 1; size++) {
			input = getchar();

			if (input == EOF && !size) {
				return -1;
			}
			if (input == EOF && size) {
				return -2;
			}
			if (input == '\n') {
				(*line)[size] = 0;
				return size;
			}

			(*line)[size] = input;
		}

		buffer_size *= 2;

		char *new_alloc = realloc(*line, buffer_size);
		if (!new_alloc) {
			exit(1);
		}
		*line = new_alloc;
	}
}


/**
 * @brief Konstruktor typu @ref command_t.
 * Alokuje pamiec na strukture.
 * 
 * @param[in] command_type  : typ polecenia, ktory tworzymy
 * 
 * @return Wskaznik na nowa, pusta strukture @ref command_t.
 */
command_t *create_new_command(command_type_t command_type) {
	command_t *new_command = malloc(sizeof(command_t));
	if (!new_command) {
		exit(1);
	}

	new_command->command_type = command_type;
	new_command->arg_count = 0;

	for (int i = 0; i < 4; i++) {
		new_command->args[i] = 0;
	}

	return new_command;
}


/**
 * @brief Zwalnia pamiec zaalokowana na polecenie @p command.
 * 
 * @param[in,out] command   : wskaznik na strukture zawierajaca dane o poleceniu
 */
void erase_command(command_t *command) {
	free(command);
}


/**
 * @brief Przetwarza napis na typ @ref command_type_t.
 * Bierze pod uwage tylko napisy odnoszace sie do polecen kontrolujacych gre.
 * 
 * @param[in] command_name  : wskaznik na poczatek napisu
 * 
 * @return Przetworzone polecenie @p command_name na typ @ref command_type_t
 * lub ERROR gdy napis nie byl poprawny.
 */
command_type_t get_command_type(char *command_name) {
	if (!strcmp(command_name, "B")) {
		return NEW_GAME_BATCH;
	}
	if (!strcmp(command_name, "I")) {
		return NEW_GAME_INTERACTIVE;
	}
	if (!strcmp(command_name, "m")) {
		return MOVE;
	}
	if (!strcmp(command_name, "g")) {
		return GOLDEN_MOVE;
	}
	if (!strcmp(command_name, "b")) {
		return BUSY_FIELDS;
	}
	if (!strcmp(command_name, "f")) {
		return FREE_FIELDS;
	}
	if (!strcmp(command_name, "q")) {
		return GOLDEN_POSSIBLE;
	}
	if (!strcmp(command_name, "p")) {
		return BOARD;
	}
	return ERROR;
}


/**
 * @brief Sprawdza, czy format wejscia jest poprawny.
 * 
 * @return Enum @ref command_type_t zawierajacy informacje o
 * poprawnosci formatu wejscia.
 * CONTINUE (instrukcja ma poprawny format)
 * ERROR/SKIP/EXIT (instrukcja ma niepoprawny format)
 **/
command_type_t check_for_invalid_input_format(char *instruction, int size) {
	if (!size) {
		return SKIP;
	}
	if (instruction[0] == '#') {
		return SKIP;
	}
	if (check_white_character(instruction[0])) {
		return ERROR;
	}
	if (size == -1) {
		return EXIT;
	}
	if (size == -2) {
		return ERROR;
	}
	if (check_for_illegal_signs(instruction, size)) {
		return ERROR;
	}
	return CONTINUE;
}


/**
 * @brief Sprawdza, czy @p input jest poprawnym zapisem liczby oraz, jesli tak
 * jest, sprawdza, czy ta liczba miesci sie w zmiennej typu uint32_t.
 * 
 * @param[in] input : wskaznik na poczatek napisu
 * 
 * @return Gdy @p input jest poprawnym zapisem liczby zwraca 1, w przeciwnym
 * wypadku 0.
 */
int is_valid_number(char *input) {
	int size = 0;
	for (char *c = input; *c != 0; c++) {
		size++;
		if (*c < '0' || *c > '9') {
			return 0;
		}
	}
	if (size > 10) {
		return 0;
	}

	uint64_t parsed_number = strtoull(input, NULL, 0);
	if (parsed_number > UINT32_MAX) {
		return 0;
	}

	return 1;
}


/**
 * @brief Przetwarza instrukcje o poprawnym formacie na typ @ref command_t.
 * 
 * @param[in] instruction   : wskaznik na poczatek instrukcji
 * 
 * @return Polecenie do wykonania.
 */
command_t *parse_command(char *instruction) {
	char *delimiters = " \t\v\f\r";
	char *pointer = strtok(instruction, delimiters);
	if (pointer == NULL) {
		return create_new_command(ERROR);
	}

	command_type_t commandType = get_command_type(pointer);
	if (commandType == ERROR) {
		return create_new_command(ERROR);
	}

	command_t *command = create_new_command(commandType);
	int count = 0;

	while ((pointer = strtok(NULL, delimiters)) != NULL) {
		if (count > 3) {
			erase_command(command);
			return create_new_command(ERROR);
		}
		if (!is_valid_number(pointer)) {
			erase_command(command);
			return create_new_command(ERROR);
		}
		command->args[count] = strtoul(pointer, NULL, 0);
		count++;
	}
	command->arg_count = count;

	return command;
}


/**
 * @brief Sprawdza, czy liczba argumentow jest poprawna dla danego polecenia.
 * 
 * @param[in] command   : wskaznik na strukture przechowujaca polecenie
 * 
 * @return Gdy polecenie ma poprawna liczba argumentow zwraca 1, w przeciwnym
 * wypadku 0.
 */
int check_correct_arguments_number(command_t *command) {
	if (command->command_type == NEW_GAME_BATCH) {
		return command->arg_count == 4;
	}
	if (command->command_type == NEW_GAME_INTERACTIVE) {
		return command->arg_count == 4;
	}
	if (command->command_type == MOVE) {
		return command->arg_count == 3;
	}
	if (command->command_type == GOLDEN_MOVE) {
		return command->arg_count == 3;
	}
	if (command->command_type == BUSY_FIELDS) {
		return command->arg_count == 1;
	}
	if (command->command_type == FREE_FIELDS) {
		return command->arg_count == 1;
	}
	if (command->command_type == GOLDEN_POSSIBLE) {
		return command->arg_count == 1;
	}
	if (command->command_type == BOARD) {
		return command->arg_count == 0;
	}
	return 0;
}


/**
 * @brief Zwraca nastepne polecenie do wykonania.
 * Alokuje pamiec na przechowanie danych.
 * Przetwarza wejscie na typ @ref command_t.
 * 
 * @return Wskaznik na strukture przechowujaca dane o poleceniu.
 */
command_t *get_command() {
	int buffer_size = BUFFER_SIZE;
	char *instruction = calloc(buffer_size, sizeof(char));
	if (!instruction) {
		exit(1);
	}
	int size = read_line(&instruction, buffer_size);

	command_type_t input_validity = check_for_invalid_input_format(
		instruction,
		size
	);
	if (input_validity != CONTINUE) {
		free(instruction);
		return create_new_command(input_validity);
	}

	command_t *command = parse_command(instruction);
	if (!check_correct_arguments_number(command)) {
		free(instruction);
		erase_command(command);
		return create_new_command(ERROR);
	}
	
	free(instruction);
	return command;
}
