#ifndef SISH_SHELL_H
#define SISH_SHELL_H

#include <stdbool.h>

pid_t
execute_command(char *cmd, bool tracing, int in_fd, int out_fd);

void
start_shell(bool tracing);

#endif