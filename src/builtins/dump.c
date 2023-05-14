#include "../shell.h"
#include "../symbol-table/symbol-table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MAX_COMMANDS 10
#define MAX_LENGTH 100
#define MAX_ALIAS_NAME_LENGTH 100
#define MAX_ALIAS_VALUE_LENGTH 200
#define MAX_ALIASES 100
#define DEFAULT_NUM_LINES 10
#define MAX_LINE_LENGTH 100

int dump(int argc, char **argv) {
    dump_local_symtab();
    return 0;
}

int pwd(int argc, char **argv) {
  char cwd[MAX_LENGTH];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("%s\n", cwd);
  } else {
    perror("pwd");
    return 1;
  }
  return 0;
}

int cd(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "cd: expected argument to \"cd\"\n");
  } else {
    if (chdir(argv[1]) != 0) {
      perror("cd");
    }
  }
  return 1;
}

char *aliases[MAX_ALIASES];

int alias(int argc, char **argv) {
  int i;
  if (argc == 1) {
    for (i = 0; i < MAX_ALIASES; i++) {
      if (aliases[i] != NULL) {
          printf("%s\n", aliases[i]);
      }
    }
  } else if (argc == 2) {
    for (i = 0; i < MAX_ALIASES; i++) {
      if (aliases[i] != NULL && strcmp(aliases[i], argv[1]) == 0) {
          printf("%s='%s'\n", argv[1], aliases[i]);
          break;
      }
    }
    if (i == MAX_ALIASES) {
      fprintf(stderr, "alias: %s: not found\n", argv[1]);
      return 1;
      }
  } else if (argc == 3) {
    for (i = 0; i < MAX_ALIASES; i++) {
      if (aliases[i] == NULL) {
        aliases[i] = malloc(strlen(argv[2]) + 1);
        strcpy(aliases[i], argv[2]);
        break;
      } else if (strcmp(aliases[i], argv[1]) == 0) {
        free(aliases[i]);
        aliases[i] = malloc(strlen(argv[2]) + 1);
        strcpy(aliases[i], argv[2]);
        break;
      }
    }
    if (i == MAX_ALIASES) {
      fprintf(stderr, "alias: too many aliases defined\n");
      return 1;
    }
  } else {
    fprintf(stderr, "alias: too many arguments\n");
    return 1;
  }
  return 0;
}


int source(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: source FILENAME\n");
    return 1;
  }
  
  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    perror("source");
    return 1;
  }
  
  char buffer[BUFSIZ];
  while (fgets(buffer, BUFSIZ, file) != NULL) {
    if (buffer[0] == '#' || buffer[0] == '\n') {
        continue; // skip comments and empty lines
    }
    system(buffer);
  }
  
  if (ferror(file)) {
    perror("source");
    fclose(file);
    return 1;
  }
  
  fclose(file);
  return 0;
}

/* Function to print the last n lines of a file */
int tail(int argc, char **argv) {
    int num_lines = DEFAULT_NUM_LINES;
    char *filename;
    FILE *fp;
    char buffer[MAX_LINE_LENGTH];
    char **lines;
    int i, line_count = 0, start_index;
    
    /* Parse command line arguments */
    if (argc > 1 && argv[1][0] == '-') {
        /* Second argument is a flag, parse it */
        if (sscanf(argv[1], "-%d", &num_lines) != 1) {
            fprintf(stderr, "Invalid argument: %s\n", argv[1]);
            return 1;
        }
        argc--;
        argv++;
    }
    
    /* Check if a filename was provided */
    if (argc == 1) {
        fprintf(stderr, "No filename provided\n");
        return 1;
    }
    filename = argv[1];
    
    /* Open the file for reading */
    fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file: %s\n", strerror(errno));
        return 1;
    }
    
    /* Allocate memory for an array of pointers to lines */
    lines = (char **)malloc(sizeof(char *) * num_lines);
    if (lines == NULL) {
        fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
        fclose(fp);
        return 1;
    }
    for (i = 0; i < num_lines; i++) {
        lines[i] = NULL;
    }
    
    /* Read the file one line at a time */
    while (fgets(buffer, MAX_LINE_LENGTH, fp) != NULL) {
        /* Allocate memory for a copy of the line */
        lines[line_count % num_lines] = strdup(buffer);
        if (lines[line_count % num_lines] == NULL) {
            fprintf(stderr, "Error allocating memory: %s\n", strerror(errno));
            fclose(fp);
            free(lines);
            return 1;
        }
        line_count++;
    }
    
    /* Determine the starting index for printing lines */
    if (line_count <= num_lines) {
        start_index = 0;
    } else {
        start_index = line_count % num_lines;
    }
    
    /* Print the last n lines */
    for (i = 0; i < num_lines; i++) {
        if (lines[(start_index + i) % num_lines] != NULL) {
            printf("%s", lines[(start_index + i) % num_lines]);
            free(lines[(start_index + i) % num_lines]);
        }
    }
    
    /* Clean up */
    fclose(fp);
    free(lines);
    
    return 0;
}
