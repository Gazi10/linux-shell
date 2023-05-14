#include "../shell.h"
#include "../symbol-table/symbol-table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_COMMANDS 10
#define MAX_LENGTH 100
#define MAX_ALIAS_NAME_LENGTH 100
#define MAX_ALIAS_VALUE_LENGTH 200
#define MAX_ALIASES 100

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
