#include "shell.h"

#include <sys/types.h>
#include <sys/wait.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void
execute_command(char *command, bool tracing)
{
    if (tracing)
        fprintf(stderr, "+ %s\n", command);

    pid_t pid = fork();
    if (pid < 0) {
        warn("fork");
        return;
    }

    if (pid == 0) {
        execlp(command, command, NULL);
        err(127, "%s", command);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1)
        warn("waitpid");
}

void
start_shell(bool tracing)
{
    char buf[sysconf(_SC_ARG_MAX)];
    while (true) {
        printf("sish$ ");
        if (fgets(buf, sizeof(buf), stdin) == NULL)
            warn("fgets");
        int buf_len = strlen(buf);
        if (buf_len <= 1)
            continue;
        buf[buf_len - 1] = 0;
        execute_command(buf, tracing);
    }
}
