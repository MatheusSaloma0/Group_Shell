#ifndef INPUT_MANAGER_H_
#define INPUT_MANAGER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> 
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * Estrutura que representa um comando.
 */
typedef struct command {
    char** args; // Armazena o nome do comando e seus argumentos.
} Command;

/**
 * Estrutura que representa uma linha de comando.
 */
typedef struct cmd_line {
    Command* array;    // Array de comandos.
    unsigned int size; // Quantidade de comandos.
} CMD_Line;

CMD_Line splitter(char *);

void free_commands(CMD_Line);

#endif