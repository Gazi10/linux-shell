#include <string.h>
#include "shell.h"
#include "symbol-table/symbol-table.h"
extern char **environ;

void init(void) {
  // Initialize symbol table
  init_symtab();
  
  // Loop through environment variables
  struct symtab_entry_s *entry;
  char **p2 = environ;
  while(*p2) {
    // Check if the environment variable has a value
    char *eq = strchr(*p2, '=');
    if(eq) {
      // Extract the name of the environment variable
      int len = eq-(*p2);
      char name[len+1];
      strncpy(name, *p2, len);
      name[len] = '\0';
      
      // Add the environment variable to the symbol table
      entry = add_to_symtab(name);
      if(entry) {
        // Set the value of the symbol table entry to the environment variable value
        symtab_entry_setval(entry, eq+1);
        // Set the export flag to indicate that the variable is an environment variable
        entry->flags |= FLAG_EXPORT;
      }
    }
    else {
      // If the environment variable does not have a value, simply add it to the symbol table
      entry = add_to_symtab(*p2);
    }
    p2++;
  }
  
  // Set default values for PS1 and PS2 variables
  entry = add_to_symtab("PS1");
  symtab_entry_setval(entry, "$ ");
  entry = add_to_symtab("PS2");
  symtab_entry_setval(entry, "> ");
}
