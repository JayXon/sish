#include "shell.h"

#include <sys/types.h>
#include <sys/wait.h>

#include <ctype.h>
#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char **
split_args(char *args)
{
    int argc = 2;
    for (int i = 2; args[i]; i++)
        if ((args[i - 1] == ' ' || args[i - 1] == '\t') && (args[i] != ' ' && args[i] != '\t'))
            argc++;

    char **argv = malloc(argc * sizeof(char *));

    argc = 0;
    const char *delim = " \t";
    for (char *token = strtok(args, delim); token; token = strtok(NULL, delim))
        argv[argc++] = token;
    argv[argc] = NULL;

    return argv;
}

char *
skip_space(char *s)
{
    while (*s == ' ' || *s == '\t')
        s++;
    return s;
}

void
execute_command(char *command, bool tracing)
{
    command = skip_space(command);
    if (tracing)
        fprintf(stderr, "+ %s\n", command);

    char **argv = split_args(command);

    pid_t pid = fork();
    if (pid < 0) {
        warn("fork");
        return;
    }

    if (pid == 0) {
        execvp(command, argv);
        err(127, "%s", command);
    }
    free(argv);

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
