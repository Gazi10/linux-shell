#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "shell.h"
#include "node.h"
#include "executor.h"

/* search for the executable file in directories specified by PATH env variable */
char *search_path(char *file) {
    /* get the PATH environment variable */
    char *PATH = getenv("PATH");
    char *p    = PATH;
    char *p2;

    /* loop through directories in the PATH */
    while(p && *p) {
        /* find the end of the current directory */
        p2 = p;
        while(*p2 && *p2 != ':') {
            p2++;
        }

        /* determine the length of the current directory */
        int plen = p2 - p;
        if(!plen) {
            /* the current directory is empty, so set its length to 1 */
            plen = 1;
        }

        /* determine the length of the filename */
        int alen = strlen(file);

        /* allocate space for the full path to the file */
        char path[plen + 1 + alen + 1];

        /* copy the directory name to the path variable */
        strncpy(path, p, p2 - p);
        path[p2 - p] = '\0';

        /* add a trailing '/' character if necessary */
        if(p2[-1] != '/') {
            strcat(path, "/");
        }

        /* append the filename to the path */
        strcat(path, file);

        /* check if the file exists and is a regular file */
        struct stat st;
        if(stat(path, &st) == 0) {
            if(!S_ISREG(st.st_mode)) {
                /* file exists but is not a regular file, so skip to next directory */
                errno = ENOENT;
                p = p2;
                if(*p2 == ':') {
                    p++;
                }
                continue;
            }

            /* allocate space for the full path to the file */
            p = malloc(strlen(path) + 1);
            if(!p) {
                /* memory allocation failed */
                return NULL;
            }

            /* copy the full path to the allocated space and return it */
            strcpy(p, path);
            return p;
        }
        else {
            /* skip to next directory */
            p = p2;
            if(*p2 == ':') {
                p++;
            }
        }
    }

    /* file not found in any directory */
    errno = ENOENT;
    return NULL;
}

// Executes a command with arguments
int do_exec_cmd(int argc, char **argv) {
    // If the first argument contains a forward slash, use it as an absolute path and execute
    if (strchr(argv[0], '/')) {
        execv(argv[0], argv);
    } 
    // Otherwise, search for the executable in the directories listed in PATH
    else {
        char *path = search_path(argv[0]);
        if (!path) {
            return 0;
        }
        // Execute the command with its full path
        execv(path, argv);
        free(path);
    }
    return 0;
}

// Frees the memory used by argv
static inline void free_argv(int argc, char **argv) {
    if (!argc) {
        return;
    }
    while (argc--) {
        free(argv[argc]);
    }
}

// Executes a simple command represented by a tree node
int do_simple_command(struct node_s *node) {
    if (!node) {
        return 0;
    }
    // The first child of the simple command is the actual command to be executed
    struct node_s *child = node->first_child;
    if (!child) {
        return 0;
    }
    
    int argc = 0;
    long max_args = 255;
    // Allocate an array to store the command's arguments
    char *argv[max_args + 1]; // Keep 1 for the terminating NULL arg
    char *str;
    
    // Extract the arguments from the tree node and store them in the argv array
    while (child) {
        str = child->val.str;
        argv[argc] = malloc(strlen(str) + 1);
        
        if (!argv[argc]) {
            // Free the argv array if memory allocation fails
            free_argv(argc, argv);
            return 0;
        }
        
        strcpy(argv[argc], str);
        if (++argc >= max_args) {
            break;
        }
        child = child->next_sibling;
    }

    argv[argc] = NULL; // The last element of argv must be NULL
    int i = 0;
    for( ; i < builtins_count; i++) {
        if(strcmp(argv[0], builtins[i].name) == 0) {
            builtins[i].func(argc, argv);
            free_argv(argc, argv);
            return 1;
        }
    }

    pid_t child_pid = 0;
    // Fork a child process to execute the command
    if ((child_pid = fork()) == 0) {
        // Execute the command in the child process
        do_exec_cmd(argc, argv);
        // If execv() returns, there was an error
        fprintf(stderr, "error: failed to execute command: %s\n", strerror(errno));
        if (errno == ENOEXEC) {
            exit(126); // The command is not a valid executable format
        } else if (errno == ENOENT) {
            exit(127); // The command was not found or is not executable
        } else {
            exit(EXIT_FAILURE);
        }
    } else if (child_pid < 0) {
        // Forking failed
        fprintf(stderr, "error: failed to fork command: %s\n", strerror(errno));
        return 0;
    }
    // Wait for the child process to terminate and free the argv array
    int status = 0;
    waitpid(child_pid, &status, 0);
    free_argv(argc, argv);
    
    return 1;
}
