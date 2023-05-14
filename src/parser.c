#include <unistd.h>
#include "shell.h"
#include "parser.h"
#include "token.h"
#include "node.h"
#include "source.h"

// This function takes a token and returns a node representing a simple command
struct node_s *parse_simple_command(struct token_s *tok) {
    if(!tok) {
        return NULL;
    }

    // Create a new command node
    struct node_s *cmd = new_node(NODE_COMMAND);
    if(!cmd) {
        // If failed to create a node, free the token and return NULL
        free_token(tok);
        return NULL;
    }

    // Get the source from the token
    struct source_s *src = tok->src;

    // Loop through all the tokens in the source
    do {
        // If the first character of the token is a newline character, stop looping
        if(tok->text[0] == '\n') {
            free_token(tok);
            break;
        }

        // Create a new node representing a word
        struct node_s *word = new_node(NODE_VAR);
        if(!word) {
            // If failed to create a node, free the command node, token and return NULL
            free_node_tree(cmd);
            free_token(tok);
            return NULL;
        }

        // Set the value of the node to the text of the token
        set_node_val_str(word, tok->text);

        // Add the word node as a child of the command node
        add_child_node(cmd, word);

        // Free the token
        free_token(tok);
    } while((tok = tokenize(src)) != &eof_token);

    // Return the command node
    return cmd;
}
