#include <errno.h>
#include "shell.h"
#include "source.h"

// Decrements the current position in the source buffer by one.
void unget_char(struct source_s *src) {
  if (src->curpos < 0) {
    return;
  }
  src->curpos--;
}

// Returns the next character in the source buffer.
char next_char(struct source_s *src) {
  // If the source or buffer is invalid, set the errno and return an error character.
  if (!src || !src->buffer) {
    errno = ENODATA;
    return ERRCHAR;
  }

  // If the current position is at the beginning, set it to -1 so that the next character read is the first one.
  char c1 = 0;
  if (src->curpos == INIT_SRC_POS) {
    src->curpos = -1;
  }
  // Otherwise, get the next character in the buffer and increment the current position.
  else {
    c1 = src->buffer[src->curpos];
  }

  // If the current position is greater than or equal to the buffer size, return EOF.
  if (++src->curpos >= src->bufsize) {
    src->curpos = src->bufsize;
    return EOF;
  }

  // Otherwise, return the next character in the buffer.
  return src->buffer[src->curpos];
}

// Returns the next character in the source buffer without advancing the current position.
char peek_char(struct source_s *src)
{
  if (!src || !src->buffer) {
    errno = ENODATA;
    return ERRCHAR;
  }

  // Get the current position in the buffer and increment it.
  long pos = src->curpos;
  if (pos == INIT_SRC_POS) {
    pos++;
  }
  pos++;

  // If the incremented position is greater than or equal to the buffer size, return EOF.
  if (pos >= src->bufsize) {
    return EOF;
  }

  // Otherwise, return the next character in the buffer.
  return src->buffer[pos];
}

// Skips over any white space characters in the source buffer.
void skip_white_spaces(struct source_s *src)
{
  char c;
  if (!src || !src->buffer) {
    return;
  }

  // Loop through the source buffer until a non-white space character is found.
  while (((c = peek_char(src)) != EOF) && (c == ' ' || c == '\t')) {
    next_char(src);
  }
}
