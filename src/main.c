#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "shell.h"
#include "source.h"
#include "parser.h"
#include "executor.h"

int main(int argc, char **argv) {
  char *user_input;
  init();
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

    // Execute the user's input command
    struct source_s src;
    src.buffer   = user_input;
    src.bufsize  = strlen(user_input);
    src.curpos   = INIT_SRC_POS;
    parse_and_execute(&src);

    
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

/**
 * This function reads commands from the input source, parses them, and
 * executes them.
 *
 * @param src The input source to read commands from.
 *
 * @return Returns 1 if the execution was successful, 0 otherwise.
 */
int parse_and_execute(struct source_s *src)
{
    // Skip leading white spaces in the input source
    skip_white_spaces(src);

    // Tokenize the input source
    struct token_s *tok = tokenize(src);

    // If there are no more tokens, return 0 indicating failure
    if(tok == &eof_token)
    {
        return 0;
    }

    // Loop over tokens and execute commands
    while(tok && tok != &eof_token)
    {
        // Parse a simple command from the current token
        struct node_s *cmd = parse_simple_command(tok);

        // If parsing fails, break out of the loop
        if(!cmd)
        {
            break;
        }

        // Execute the parsed command
        do_simple_command(cmd);

        // Free the memory used by the command node
        free_node_tree(cmd);

        // Tokenize the next command
        tok = tokenize(src);
    }

    // Return 1 indicating successful execution
    return 1;
}

