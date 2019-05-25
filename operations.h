#ifndef OPERATIONS_H_
#define OPERATIONS_H_

#include <unistd.h> 
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "input_manager.h"

void gsh_handler_SIGINT(int sig);

void gsh_handler_SIGTSTP(int sig);

void handler_SIGUSR1(int sig);

void run(CMD_Line cmds);

#endif