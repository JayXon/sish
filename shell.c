#include "shell.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef ARG_MAX
#define ARG_MAX sysconf(_SC_ARG_MAX)
#endif

static void
echo(char **argv, int fd)
{
    if (*++argv)
        dprintf(fd, "%s", *argv++);
    while (*argv)
        dprintf(fd, " %s", *argv++);
    dprintf(fd, "\n");
}

static void
cd(char *path)
{
    if (path == NULL)
        if ((path = getenv("HOME")) == NULL) {
            struct passwd *pw = getpwuid(getuid());
            if (pw == NULL) {
                warn("getpwuid");
                return;
            }
            if ((path = pw->pw_dir) == NULL)
                return;
        }

    if (chdir(path) == -1)
        warn("cd: %s", path);
}

char **
split_args(char *args)
{
    int argc = 2;
    for (int i = 1; args[i]; i++)
        if (args[i] == ' ')
            argc++;

    char **argv = malloc(argc * sizeof(char *));
    if (argv == NULL)
        err(EXIT_FAILURE, "malloc");

    argc = 0;
    for (char *token = strtok(args, " "); token; token = strtok(NULL, " "))
        argv[argc++] = token;
    argv[argc] = NULL;

    return argv;
}

static char *
skip_space(char *s)
{
    while (*s == ' ' || *s == '\t')
        s++;
    return s;
}

static void
shrink_space(char *s)
{
    char *p = s;
    s = skip_space(s);
    while (*s) {
        if (*s == ' ' || *s == '\t') {
            s = skip_space(s);
            *p++ = ' ';
        }
        else
            *p++ = *s++;
    }
    *p-- = 0;
    if (*p == ' ')
        *p = 0;
}

static int
open_redirect_file(char **file_name, int fd, int open_flag)
{
    char *p = skip_space(*file_name);
    int l = strcspn(p, " \t\n<>");
    if (l == 0) {
        warnx("Redirection file name needed");
        return -1;
    }
    char t = p[l];
    p[l] = 0;
    if (fd != STDIN_FILENO && fd != STDOUT_FILENO)
        close(fd);
    if ((fd = open(p, open_flag, 0644)) == -1)
        warn("%s", p);

    for (int i = 0; i < l; i++)
        p[i] = ' ';
    p[l] = t;
    *file_name = p + l - 1;
    return fd;
}

void
execute_command(char *cmd, bool tracing, int in_fd, int out_fd)
{
    for (char *p = cmd; *p; p++) {
        if (*p == '>') {
            int flags = O_WRONLY | O_CREAT | O_TRUNC;
            *p++ = ' ';
            if (*p == '>') {
                *p++ = ' ';
                flags = O_WRONLY | O_CREAT | O_APPEND;
            }
            if ((out_fd = open_redirect_file(&p, out_fd, flags)) == -1)
                return;
        } else if (*p == '<') {
            *p++ = ' ';
            if ((in_fd = open_redirect_file(&p, in_fd, O_RDONLY)) == -1)
                return;
        }
    }
    shrink_space(cmd);
    if (*cmd) {
        if (tracing)
            fprintf(stderr, "+ %s\n", cmd);

        char **argv = split_args(cmd);

        if (strcmp(cmd, "exit") == 0)
            exit(EXIT_SUCCESS);
        else if (strcmp(cmd, "echo") == 0)
            echo(argv, out_fd);
        else if (strcmp(cmd, "cd") == 0)
            cd(argv[1]);
        else {
            pid_t pid = fork();
            if (pid < 0) {
                warn("fork");
                return;
            }

            if (pid == 0) {
                dup2(in_fd, STDIN_FILENO);
                dup2(out_fd, STDOUT_FILENO);
                execvp(cmd, argv);
                err(127, "%s", cmd);
            }

            int status;
            if (waitpid(pid, &status, 0) == -1)
                warn("waitpid");
        }
        free(argv);
    }
    if (in_fd != STDIN_FILENO)
        close(in_fd);
    if (out_fd != STDOUT_FILENO)
        close(out_fd);
}

void
start_shell(bool tracing)
{
    char buf[ARG_MAX];
    while (true) {
        printf("sish$ ");
        if (fgets(buf, sizeof(buf), stdin) == NULL)
            warn("fgets");
        else {
            char *cmd = strtok(buf, "|\n");
            int pipefd[2], in_fd = STDIN_FILENO, out_fd;
            while (cmd) {
                char *next_cmd = strtok(NULL, "|\n");
                if (next_cmd) {
                    if (pipe(pipefd) == -1) {
                        warn("pipe");
                        break;
                    }
                    out_fd = pipefd[1];
                } else
                    out_fd = STDOUT_FILENO;
                execute_command(cmd, tracing, in_fd, out_fd);
                in_fd = pipefd[0];
                cmd = next_cmd;
            }
        }
    }
}
