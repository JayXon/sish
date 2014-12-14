#ifndef SISH_SHELL_H
#define SISH_SHELL_H

#include <stdbool.h>

extern int status;

void
parse_command(char *cmd, bool tracing);

void
start_shell(bool tracing);

#endif