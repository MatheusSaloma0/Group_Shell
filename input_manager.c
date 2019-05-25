#include "input_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static const char sep[3] = "->";        // Separador de comandos.
static const unsigned int max_cmd = 5;  // Número máximo de comandos em um input.
static const unsigned int max_args = 3; // Número máximo de argumentos de um comando.

/**
 * Libera a memória alocada para a struct CMD_Line.
 */
void free_commands(CMD_Line cmds){
    for(int i = 0; i < cmds.size; i++){
        for(int j = 0; cmds.array[i].args[j] != NULL; j++){
            free(cmds.array[i].args[j]);
        }
    }
    free(cmds.array);
}

/**
 * Separa os argumentos de um comando.
 */
char** args_separator(char* input){
    char** args = malloc((max_args+2)*sizeof(char*));//Nome do comando + Argumentos + NULL 
    char delim[2] = " ";
    int i = 0;
    char* token = strtok(input,delim); // Separa o input até a ocorrência do primeiro delimitador.
    while(token != NULL && i < max_args+1){
        args[i] = malloc(strlen(token)*sizeof(char*)); // Aloca a memória para armazenar o token.
        strcpy(args[i++], token); // Armazena o token em args.
        token = strtok(NULL,delim); // Separa o input até a ocorrência do próximo delimitador.
    }
    // Inserimos NULL na última posição de args,
    // pois é necessário para posteriormente utilizarmos args na execvp().
    args[i] = NULL; 
    return args;
}

/**
 * Recebe a linha de comando (string), separa-os e retorna uma CMD_Line.
 */
CMD_Line splitter(char* input){
    Command* c = malloc(max_cmd*sizeof(Command));
    int i = 0; // Contador de comandos.
    char* token; 

    while((token = strstr(input, sep)) && (i < max_cmd)){ // Enquanto existirem comandos "separados":
        int sz = strlen(input)-strlen(token); // sz é o tamanho da string do comando.

        // Ajuste do ponteiro input e do tamanho sz quando há
        // espaço nas extremidades da string.
        while(input[0]    == ' ' && sz > 0){ input++; sz--; } 
        while(input[sz-1] == ' ' && sz > 0) sz--;            
      
        if(sz == 0){ // Verifica se é um comando vazio. Ex: c1 ->  -> c2
            printf("gsh: erro de sintaxe próximo do 'token' não esperado '%s'\n", sep);
            free_commands((CMD_Line){c,i});
            return (CMD_Line){NULL,0};
        } else {
            char aux[sz];
            strncpy(aux, input, sz);
            aux[sz] = '\0';
            c[i++].args = args_separator(aux);
        }
        input = token + strlen(sep); // Ajuste do ponteiro input para depois do separador.
    }
    if(i != max_cmd){
        while(input[0] == ' '){ input++; }
        if(input[0] != '\0'){
            c[i++].args = args_separator(input);
        }
    }
    return (CMD_Line){c,i};
}