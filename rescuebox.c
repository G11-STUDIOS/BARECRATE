#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>

#define RB_MAX_INPUT 4096
#define RB_MAX_ARGS 256

static void print_help(void)
{
    printf("RescueBox\n");
    printf("Programs are loaded from ./bin/\n");
    printf("\n");
    printf("Built-in commands:\n");
    printf("  cd [dir]    Change directory\n");
    printf("  help        Show this message\n");
    printf("  exit        Exit RescueBox\n");
}

static int change_directory(const char *dir)
{
    char path[PATH_MAX];
    const char *target = dir;


    if (dir == NULL || strcmp(dir, "~") == 0) {
        target = getenv("HOME");

        if (target == NULL || target[0] == '\0') {
            target = "/";
        }
    }

    else if (strncmp(dir, "~/", 2) == 0) {
        const char *home = getenv("HOME");

        if (home == NULL || home[0] == '\0') {
            home = "/";
        }

        int written = snprintf(
            path,
            sizeof(path),
            "%s/%s",
            home,
            dir + 2
        );

        if (written < 0 || (size_t)written >= sizeof(path)) {
            fprintf(stderr, "cd: path too long\n");
            return -1;
        }

        target = path;
    }

    if (chdir(target) != 0) {
        fprintf(stderr, "cd: %s: %s\n", target, strerror(errno));
        return -1;
    }

    return 0;
}

int main(void)
{
    char exe_path[PATH_MAX];
    char bin_path[PATH_MAX];

    ssize_t len = readlink(
        "/proc/self/exe",
        exe_path,
        sizeof(exe_path) - 1
    );

    if (len < 0) {
        perror("RescueBox: cannot determine executable path");
        return 1;
    }

    exe_path[len] = '\0';

    char *slash = strrchr(exe_path, '/');

    if (slash == NULL) {
        fprintf(
            stderr,
            "RescueBox: cannot determine executable directory\n"
        );
        return 1;
    }

    *slash = '\0';

    int written = snprintf(
        bin_path,
        sizeof(bin_path),
        "%s/bin",
        exe_path
    );

    if (written < 0 || (size_t)written >= sizeof(bin_path)) {
        fprintf(stderr, "RescueBox: bin path is too long\n");
        return 1;
    }

    char input[RB_MAX_INPUT];

    while (1) {
        printf("> ");
        fflush(stdout);


        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }


        input[strcspn(input, "\n")] = '\0';


        if (input[0] == '\0')
            continue;

        char *args[RB_MAX_ARGS];
        int arg_count = 0;

        char *token = strtok(input, " \t");

        while (token != NULL && arg_count < RB_MAX_ARGS - 1) {
            args[arg_count++] = token;
            token = strtok(NULL, " \t");
        }

        args[arg_count] = NULL;

        if (arg_count == 0)
            continue;

        /*
         * Built-in: exit
         */
        if (strcmp(args[0], "exit") == 0) {
            break;
        }


        if (strcmp(args[0], "help") == 0) {
            print_help();
            continue;
        }


        if (strcmp(args[0], "cd") == 0) {
            if (arg_count > 2) {
                fprintf(stderr, "cd: too many arguments\n");
            } else {
                change_directory(
                    arg_count == 2 ? args[1] : NULL
                );
            }

            continue;
        }

        char program[PATH_MAX];

        written = snprintf(
            program,
            sizeof(program),
            "%s/%s",
            bin_path,
            args[0]
        );

        if (written < 0 || (size_t)written >= sizeof(program)) {
            fprintf(
                stderr,
                "%s: command path too long\n",
                args[0]
            );
            continue;
        }


        if (access(program, X_OK) != 0) {
            printf("%s: command not found\n", args[0]);
            continue;
        }


        pid_t pid = fork();

        if (pid < 0) {
            perror("RescueBox: fork");
            continue;
        }

        if (pid == 0) {
            execv(program, args);

     
            fprintf(
                stderr,
                "RescueBox: %s: %s\n",
                args[0],
                strerror(errno)
            );

            _exit(127);
        }


        int status;

        if (waitpid(pid, &status, 0) < 0) {
            perror("RescueBox: waitpid");
        }
    }

    return 0;
}
