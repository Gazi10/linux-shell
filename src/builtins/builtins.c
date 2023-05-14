#include "../shell.h"
struct builtin_s builtins[] = {   
  { "dump" , dump },
  { "pwd", pwd },
  { "cd", cd },
  { "alias", alias },
  { "source", source },
  { "tail", tail }
};
int builtins_count = sizeof(builtins)/sizeof(struct builtin_s);
