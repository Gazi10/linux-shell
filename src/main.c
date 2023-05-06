#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "shell.h"

int main(int argc, char **argv) {
  char *user_input;

  while (1) {
    // Display a shell prompt to the user
    print_prompt1();

    // Read the user's input command
    user_input = read_cmd();

    // If the user input is null, exit the program
    if (!user_input) {
      exit(EXIT_SUCCESS);
    }

    // If the user input is empty or just a new line character, continue to the next iteration of the loop
    if (user_input[0] == '\0' || strcmp(user_input, "\n") == 0) {
      free(user_input);
      continue;
    }

    // If the user input is "exit", free the memory and exit the loop
    if (strcmp(user_input, "exit\n") == 0) {
      free(user_input);
      break;
    }

    // Display the user's input command to the console
    printf("%s\n", user_input);
    
    // Free the memory used by the user input
    free(user_input);
  }

  // Exit the program with a success status code
  exit(EXIT_SUCCESS);
}

char *read_cmd(void) {
  char buf[1024];
  char *ptr = NULL;
  char ptrlen = 0;

  // Loop to read input from the console
  while(fgets(buf, 1024, stdin)) {

    // Determine length of input string
    int buflen = strlen(buf);

    // Allocate memory for the input string
    if(!ptr) {
      ptr = malloc(buflen+1);
    }
    else {
      char *ptr2 = realloc(ptr, ptrlen+buflen+1);

      // Reallocate memory for the input string
      if(ptr2) {
        ptr = ptr2;
      }
      else {
        free(ptr);
        ptr = NULL;
      }
    }

    // Handle memory allocation errors
    if(!ptr) {
      fprintf(stderr, "error: failed to alloc buffer: %s\n", 
              strerror(errno));
      return NULL;
    }

    // Append input to the input string
    strcpy(ptr+ptrlen, buf);

    // Check if input string ends with a newline character
    if(buf[buflen-1] == '\n') {

      // If input string ends with a newline character, check if it was escaped
      if(buflen == 1 || buf[buflen-2] != '\\') {

        // If input string was not escaped, return the input string
        return ptr;
      }

      // If input string was escaped, remove the newline character and print the second prompt
      ptr[ptrlen+buflen-2] = '\0';
      buflen -= 2;
      print_prompt2();
    }
    // Update the length of the input string
    ptrlen += buflen;
  }
  // Return the input string
  return ptr;
}

