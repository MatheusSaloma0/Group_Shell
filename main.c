#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> 
#include <signal.h>
#include <errno.h>
#include "input_manager.h"
#include "operations.h"

#define SZ 100

/**
 * Primeiro trabalho de Sistemas Operacionais 2019/1.
 *   gsh (group shell)
 * 
 * Grupo:
 *   Leonardo Khoury Picoli
 *   Matheus Gomes Arante de Souza
 *   Matheus Salomão
 */ 

int main(){
    pid_t session = setsid();

    // Modifica o tratamento de alguns sinais.
    signal(SIGTSTP, gsh_handler_SIGTSTP);
    signal(SIGUSR2, SIG_IGN);
    signal(SIGUSR1, handler_SIGUSR1);
    signal(SIGINT, gsh_handler_SIGINT);

    char* input = malloc(SZ*sizeof(char));

    while(true){
        printf("\ngsh> ");
        fgets(input, SZ, stdin);
        
        input[strlen(input)-1] = '\0'; // "Remove" o '\n' de input.
    
        if(strlen(input) == 0) continue;
        
        CMD_Line cmds = splitter(input);
        if(cmds.size == 0) continue;
    
        run(cmds);
        
        free_commands(cmds);
    }
    free(input);

    return 0;
}
