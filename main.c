/*
 * sish
 *
 * Advanced Programming in the UNIX Environment - Final Assignment
 * http://www.cs.stevens.edu/~jschauma/631/f14-sish.html
 *
 */

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "shell.h"

static void
usage(void)
{
    (void)fprintf(stderr, "Usage: sish [-x] [-c command]\n");
    exit(EXIT_FAILURE);
}


int
main(int argc, char *argv[])
{
    bool tracing = false;
    char *command = NULL;

    char opt;
    while ((opt = getopt(argc, argv, "xc:")) != -1) {
        switch (opt) {
        case 'x':
            tracing = true;
            break;
        case 'c':
            command = optarg;
            break;
        default:
            usage();
        }
    }

    if (argc > optind)
        usage();

    char *shell_path = realpath(argv[0], NULL);
    if (shell_path == NULL)
        warn("realpath");
    else if (setenv("SHELL", shell_path, 1) == -1)
        warn("setenv");
    free(shell_path);

    if (command)
        parse_command(command, tracing);
    else
        start_shell(tracing);

    return status;
}
