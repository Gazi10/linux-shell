#include <stdio.h>
#include "shell.h"
#include "symbol-table/symbol-table.h"

// Define a function named print_prompt1, which takes no arguments and returns void
void print_prompt1(void) {   
    // Get the symbol table entry for the PS1 variable
    struct symtab_entry_s *entry = get_symtab_entry("PS1");
    
    // If the PS1 variable is defined and has a value
    if(entry && entry->val) {
        // Print the value of the PS1 variable to stderr
        fprintf(stderr, "%s", entry->val);
    } else {
        // Print the default prompt string "$ " to stderr
        fprintf(stderr, "$ ");
    }
}

// Function that displays the second shell prompt
void print_prompt2(void) {
    // Print "> " to the console
    printf("> ");
}
