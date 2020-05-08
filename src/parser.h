#ifndef PARSER_H
#define PARSER_H


#include <stdint.h>


/**
 * Wstepny rozmiar bufora.
 */
#define BUFFER_SIZE 64


/**
 * @brief Reprezentacja typu polecenia.
 * SKIP (polecenie jest prawidlowe, ale go nie wykonujemy)
 * EXIT (sygnal do skonczenia pracy programu)
 * CONTINUE (wykorzystywany do przetwarzania blednego wejscia)
 */
typedef enum command_type {
	NEW_GAME_BATCH,
	NEW_GAME_INTERACTIVE,
	MOVE,
	GOLDEN_MOVE,
	BUSY_FIELDS,
	FREE_FIELDS,
	GOLDEN_POSSIBLE,
	BOARD,
	ERROR,
	SKIP,
	EXIT,
	CONTINUE
} command_type_t;


/**
 * @brief Struktura przechowujaca dane o poleceniu.
 * 
 * @param command   : typ polecenia do wykonania
 * @param argCount  : liczba argumentow polecenia
 * @param args      : argumenty polecenia
 */
typedef struct command {
	command_type_t command_type;
	int arg_count;
	int32_t args[4];
} command_t;


/**
 * @brief Zwraca nastepne polecenie do wykonania.
 * Alokuje pamiec na przechowanie danych.
 * Przetwarza wejscie na typ @ref command_t.
 * 
 * @return Wskaznik na strukture przechowujaca dane o poleceniu.
 */
command_t *get_command();


/**
 * @brief Zwalnia pamiec zaalokowana w funkcji @ref getCommand.
 * 
 * @param[in] command   : polecenie, ktorego zajmowana pamiec zwalniamy
 */
void erase_command(command_t *command);


#endif /* PARSER_H */
