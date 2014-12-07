#ifndef SISH_SHELL_H
#define SISH_SHELL_H

#include <stdbool.h>

void
execute_command(char *command, bool tracing);

void
start_shell(bool tracing);

#endif