#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../shell.h"
#include "../node.h"
#include "../parser.h"
#include "symbol-table.h"

struct symtab_stack_s symtab_stack; // symbol table stack used by the shell
int symtab_level; // symbol table level

void init_symtab(void) {
  symtab_stack.symtab_count = 1; // set symbol table count to 1
  symtab_level = 0; // set symbol table level to 0

  // allocate memory for the global symbol table
  struct symtab_s *global_symtab = malloc(sizeof(struct symtab_s));
  if (!global_symtab) {
    fprintf(stderr, "fatal error: no memory for global symbol table\n");
    exit(EXIT_FAILURE);
  }
  // zero out the memory and set the pointers and level of the global symbol table
  memset(global_symtab, 0, sizeof(struct symtab_s));
  symtab_stack.global_symtab = global_symtab;
  symtab_stack.local_symtab = global_symtab;
  symtab_stack.symtab_list[0] = global_symtab;
  global_symtab->level = 0;
}

// Allocate memory for a new symbol table and initialize it
struct symtab_s *new_symtab(int level) {
  // Allocate memory for the symbol table
  struct symtab_s *symtab = malloc(sizeof(struct symtab_s));
  if(!symtab) {
    // If memory allocation fails, print an error message and exit the program
    fprintf(stderr, "fatal error: no memory for new symbol table\n");
    exit(EXIT_FAILURE);
  }

  // Initialize the symbol table by setting all its values to 0
  memset(symtab, 0, sizeof(struct symtab_s));

  // Set the level of the symbol table
  symtab->level = level;

  // Return a pointer to the symbol table
  return symtab;
}

void free_symtab(struct symtab_s *symtab) {
  if(symtab == NULL) {
    return;
  }
  // free each entry in the symbol table
  struct symtab_entry_s *entry = symtab->first;
  while(entry) {
    if(entry->name) {
      free(entry->name);
    }
    if(entry->val) {
      free(entry->val);
    }
    if(entry->func_body) {
      free_node_tree(entry->func_body);
    }
    struct symtab_entry_s *next = entry->next; // get the next entry
    free(entry); // free the current entry
    entry = next; // set entry to the next entry
  }
  free(symtab); // free the symbol table itself
}

void dump_local_symtab(void) {
  // Get a pointer to the current local symbol table
  struct symtab_s *symtab = symtab_stack.local_symtab;
  
  // Set up variables for formatting the output
  int i = 0;
  int indent = symtab->level * 4;
  
  // Print out some header information for the symbol table
  fprintf(stderr, "%*sSymbol table [Level %d]:\r\n", indent, " ", symtab->level);
  fprintf(stderr, "%*s===========================\r\n", indent, " ");
  fprintf(stderr, "%*s  No               Symbol                    Val\r\n", indent, " ");
  fprintf(stderr, "%*s------ -------------------------------- ------------\r\n", indent, " ");
  
  // Loop through each entry in the symbol table and print out its name and value
  struct symtab_entry_s *entry = symtab->first;
  while(entry) {
    fprintf(stderr, "%*s[%04d] %-32s '%s'\r\n", indent, " ",
            i++, entry->name, entry->val);
    entry = entry->next;
  }
  // Print out a separator line at the end
  fprintf(stderr, "%*s------ -------------------------------- ------------\r\n", indent, " ");
}

struct symtab_entry_s *add_to_symtab(char *symbol) {
  // Check for invalid symbol name
  if(!symbol || symbol[0] == '\0') {
    return NULL;
  }

  // Get the local symbol table
  struct symtab_s *st = symtab_stack.local_symtab;
  struct symtab_entry_s *entry = NULL;

  // Check if the symbol already exists in the local symbol table
  if((entry = do_lookup(symbol, st))) {
    return entry;
  }

  // Allocate memory for a new symbol table entry
  entry = malloc(sizeof(struct symtab_entry_s));
  if(!entry) {
    // Handle memory allocation failure
    fprintf(stderr, "fatal error: no memory for new symbol table entry\n");
    exit(EXIT_FAILURE);
  }

  // Initialize the new symbol table entry
  memset(entry, 0, sizeof(struct symtab_entry_s));
  entry->name = malloc(strlen(symbol)+1);
  if(!entry->name) {
    // Handle memory allocation failure
    fprintf(stderr, "fatal error: no memory for new symbol table entry\n");
    exit(EXIT_FAILURE);
  }
  strcpy(entry->name, symbol);

  // Add the new symbol table entry to the end of the local symbol table
  if(!st->first) {
    st->first      = entry;
    st->last       = entry;
  }
  else {
    st->last->next = entry;
    st->last       = entry;
  }
  // Return the new symbol table entry
  return entry;
}

int rem_from_symtab(struct symtab_entry_s *entry, struct symtab_s *symtab) {
  int res = 0;
  // free any memory allocated to the entry
  if(entry->val) {
    free(entry->val);
  }
  if(entry->func_body) {
    free_node_tree(entry->func_body);
  }
  free(entry->name);
  // remove the entry from the symbol table
  if(symtab->first == entry) {
    symtab->first = symtab->first->next;
    if(symtab->last == entry) {
      symtab->last = NULL;
    }
    res = 1;
  }
  else {
    struct symtab_entry_s *e = symtab->first;
    struct symtab_entry_s *p = NULL;
    while(e && e != entry) {
      p = e;
      e = e->next;
    }
    if(e == entry) {
      p->next = entry->next;
      res = 1;
    }
  }
  // free the memory allocated to the entry
  free(entry);
  return res;
}

// Definition of function get_symtab_entry
struct symtab_entry_s *get_symtab_entry(char *str) {
  // Start searching from the most recent symbol table
  int i = symtab_stack.symtab_count-1;
  do {
    // Get the current symbol table
    struct symtab_s *symtab = symtab_stack.symtab_list[i];
    // Look up the given symbol in the current symbol table
    struct symtab_entry_s *entry = do_lookup(str, symtab);
    // If the symbol was found, return its entry
    if(entry) {
      return entry;
    }
    // If the symbol was not found, continue searching in the previous symbol table
  } while(--i >= 0);
  // If the symbol was not found in any symbol table, return NULL
  return NULL;
}

// Define the function with input parameters of a string and a symbol table
struct symtab_entry_s *do_lookup(char *str, struct symtab_s *symtable) {
  // If the string or symbol table is NULL, return NULL
  if(!str || !symtable) {
    return NULL;
  }
  // Set the entry to be the first entry in the symbol table
  struct symtab_entry_s *entry = symtable->first;
  // Loop through each entry in the symbol table
  while(entry) {
    // If the name of the current entry matches the string, return the entry
    if(strcmp(entry->name, str) == 0) {
        return entry;
    }
    // Move to the next entry
    entry = entry->next;
  }
  // If no entry was found with the same name as the string, return NULL
  return NULL;
}

void symtab_entry_setval(struct symtab_entry_s *entry, char *val) {
  // If the entry already has a value, free the memory allocated for it
  if(entry->val) {
    free(entry->val);
  }
  
  // If the new value is NULL, set the entry's value to NULL and return
  if(!val) {
    entry->val = NULL;
    return;
  }
  else {
    // Allocate memory for the new value, with length of the string plus 1 for the null terminator
    char *val2 = malloc(strlen(val)+1);
    // If memory allocation is successful, copy the new value to the allocated memory and set the entry's value to the new memory location
    if(val2) {
      strcpy(val2, val);
      entry->val = val2;
    }
    else {
      // If memory allocation fails, print an error message to stderr
      fprintf(stderr, "error: no memory for symbol table entry's value\n");
    }
  }
}

// This function adds a symbol table to the symbol table stack
void symtab_stack_add(struct symtab_s *symtab) {
  symtab_stack.symtab_list[symtab_stack.symtab_count++] = symtab; // Add the symbol table to the stack
  symtab_stack.local_symtab = symtab; // Set the local symbol table to the newly added symbol table
}

// This function creates a new symbol table and adds it to the symbol table stack
struct symtab_s *symtab_stack_push(void) {
  struct symtab_s *st = new_symtab(++symtab_level); // Create a new symbol table with an incremented symbol table level
  symtab_stack_add(st); // Add the symbol table to the stack
  return st; // Return the new symbol table
}

// This function removes the top symbol table from the symbol table stack and returns it
struct symtab_s *symtab_stack_pop(void) {
    if(symtab_stack.symtab_count == 0) {
      return NULL; // Return null if there are no symbol tables in the stack
    }
    struct symtab_s *st = symtab_stack.symtab_list[symtab_stack.symtab_count-1]; // Get the top symbol table from the stack
    symtab_stack.symtab_list[--symtab_stack.symtab_count] = NULL; // Remove the top symbol table from the stack
    symtab_level--; // Decrement the symbol table level
    if(symtab_stack.symtab_count == 0) {
      symtab_stack.local_symtab  = NULL; // Set the local symbol table to null
      symtab_stack.global_symtab = NULL; // Set the global symbol table to null
    }
    else {
      symtab_stack.local_symtab = symtab_stack.symtab_list[symtab_stack.symtab_count-1]; // Set the local symbol table to the next symbol table in the stack
    }
    return st; // Return the top symbol table that was removed from the stack
}

// This function returns the current local symbol table
struct symtab_s *get_local_symtab(void) {
  return symtab_stack.local_symtab; // Return the local symbol table from the symbol table stack
}

// This function returns the current global symbol table
struct symtab_s *get_global_symtab(void) {
  return symtab_stack.global_symtab; // Return the global symbol table from the symbol table stack
}

// This function returns a pointer to the symbol table stack
struct symtab_stack_s *get_symtab_stack(void) {
  return &symtab_stack; // Return a pointer to the symbol table stack
}

