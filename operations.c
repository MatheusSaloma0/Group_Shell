#include "operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> 
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "input_manager.h"

pid_t back = -2;
pid_t fore;

// Tratamento do SIGTSTP para a gsh.
void gsh_handler_SIGTSTP(int sig){ /* DO NOTHING */ }

// Tratamento do SIGTSTP para o holder.
void handler_SIGTSTP(int sig){
    if(back != -2) kill(-back, SIGTSTP); // Suspende os processos de Background.
    kill(getppid(), SIGUSR1); // Desbloqueia a gsh. 
    raise(SIGSTOP); // Se suspende.
} 

// Tratamento para desbloquear a gsh.
void handler_SIGUSR1(int sig){ /* DO NOTHING */ }

// Tratamento para o holder enviar SIGKILL para seus descendentes e ele mesmo.
void handler_SIGUSR2(int sig){
    if(back != -2) kill(-back, SIGKILL);
    kill(fore,SIGKILL);
    raise(SIGKILL);
}      

// Tratamento do SIGINT para a gsh          
void gsh_handler_SIGINT(int sig){
    int i = waitpid(-1,NULL,WNOHANG);
    while(i > 0) i = waitpid(-1,NULL,WNOHANG); // Libera os processos Zombie.

    if(i == 0){
        printf("A gsh tem filhos ainda, nao vai morrer.\n");
    } else {
        printf("\n");
        exit(0);
    }
}

/**
 * Verifica se o comando inserido é um comando interno da gsh (exit ou mywait).
 */
bool intern_commands(Command cmd,int size){
    // Para um comando interno ser executado, é necessário que este esteja
    // sozinho na linha de comando.
    if ((strcmp(cmd.args[0],"exit") == 0 || strcmp(cmd.args[0],"mywait") == 0) && size > 1){
        printf("Comandos internos devem ser executados sozinhos.\n");
        return true;
    }           
    if(strcmp(cmd.args[0],"exit") == 0){ 
        // Envia SIGCONT para que os processos que estão Stopped sejam 
        // desbloqueados e recebam o SIGUSR2.           
        kill(0,SIGCONT); 
        kill(0,SIGUSR2); // Mata todos os processos descendentes da gsh.       
        while(waitpid(-1,NULL,WNOHANG)!= -1); // Libera os processos Zombie.
        exit(0);
        return true;
    }
    if(strcmp(cmd.args[0],"mywait") == 0){ 
        //Libera todos os processos no estado Zombie.
        int tmp = waitpid(-1,NULL,WNOHANG);
        while(tmp != 0 && tmp != -1){
            tmp = waitpid(-1,NULL,WNOHANG);
        }
        return true;
    }
    return false;
}

/**
 * Verifica se o processo que enviou um sinal ao Holder pertence
 * ao grupo de foreground ou background. 
 */
void checkProcessGroup(CMD_Line cmds, pid_t sig){
    if(sig == fore){
        kill(getppid(), SIGUSR1); // Desbloqueia a gsh.
    } else {
        kill(-back,SIGKILL); // Mata todos os processos irmãos.
        for(int i = 0; i < cmds.size-2; i++){
            waitpid(-1,NULL,0);
        }       
    }       
}

/**
 * Executa o i-ésimo processo da linha de comando.
 */
void execProcess(CMD_Line cmds, int i){
    execvp(cmds.array[i].args[0], cmds.array[i].args);
    printf("Comando não reconhecido: %s\n", cmds.array[i].args[0]);
    exit(0);
}

/**
 * Executa um processo de Foreground. 
 */
void execForegroundProcess(CMD_Line cmds){
    fore = fork(); //Cria um novo processo de Foreground.
    if(fore == 0) execProcess(cmds, 0);
}

/** 
 * Executa os processos de Background.
 */
void execBackgroundProcess(CMD_Line cmds){
    for(int i = 1; i < cmds.size; i++){ 
        pid_t pid = fork(); // Cria um novo processo de Background.
        if(pid == 0){ // Filho de background
            // if(i != 1) setpgid(getpid(),back); // Adiciona os processos irmãos ao grupo de background.
            execProcess(cmds,i);
            
        }else{//HOLDER v
            if(i == 1){    
                // Armazena o pid do primeiro filho de background, que por sua vez,
                // representa o pgid de seu respectivo grupo de background.
                back = pid;
            }
            setpgid(pid, back); // Adiciona os processos irmãos ao grupo de background.
        }
    }
}

/**
 * Realiza a execução e o tratamento do fluxo dos processos
 * a partir de uma linha de comando.
 */
void run(CMD_Line cmds){ 
    if(intern_commands(cmds.array[0], cmds.size)) return;
    pid_t holder = fork(); // Cria um processo Holder.
    
    if(holder != 0){ // gsh
        // Enquanto a gsh está suspensa esperando pela morte do processo de
        // Foreground, uma mascara de bits é setada para que ela bloqueie todos 
        // os sinais com exceção do SIGUSR1 (responsável por desbloqueá-la)
        // e do SIGTSTP.
        sigset_t mask,oldmask;
        sigfillset(&mask);
        sigdelset(&mask,SIGUSR1);
        sigdelset(&mask,SIGTSTP);
        sigprocmask(SIG_BLOCK,&mask,&oldmask);
        signal(SIGINT,SIG_IGN);
        sigsuspend(&mask);
        sigprocmask(SIG_SETMASK,&oldmask,NULL);
        signal(SIGINT,gsh_handler_SIGINT);

    } else { //Holder
        // Máscara de bits para que o holder bloqueie o SIGINT.
        sigset_t mask,oldmask;
        sigemptyset(&mask);
        sigaddset(&mask,SIGINT);
        sigprocmask(SIG_BLOCK,&mask,&oldmask);

        execForegroundProcess(cmds);
        execBackgroundProcess(cmds);

        signal(SIGTSTP,handler_SIGTSTP);
        signal(SIGUSR2,handler_SIGUSR2);

        // Após todos os filhos terem sido criados,
        // o holder é suspenso até que algum filho morra.
        pid_t child_pid = waitpid(-1,NULL,0);
        
        // Verifica o grupo do processo child_pid.
        checkProcessGroup(cmds, child_pid); 
        
        if(cmds.size > 1){ // Se houver mais do que 1 comando ...
            child_pid = waitpid(-1,NULL,0); // O holder é suspenso até que algum processo morra.
            checkProcessGroup(cmds, child_pid); // Verifica o grupo do processo child_pid.
        }
        exit(0);
    }
}