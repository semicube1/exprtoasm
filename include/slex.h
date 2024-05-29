#ifndef SLEX_H_
#define SLEX_H_

#include <sys/types.h>

#define SLEX_INVALID_TOK_TYPE -1
#define SLEX_END_TOK_TYPE     -2

#define INIT_TOK_KINDS_CAP    64
#define LEXEME_MAX_LEN        64

typedef struct {
  struct {
    char type[LEXEME_MAX_LEN + 1];
    char *re;
  } *token_kinds;
  size_t token_kinds_count;
  size_t token_kinds_cap;

  char *source_str;
  size_t position;
} sLex;

// Don't want to deal with alloc/free-ing sToks
typedef struct {
  char type[LEXEME_MAX_LEN + 1];
  char lexeme[LEXEME_MAX_LEN + 1];
} sTok;

sLex *slex_new(void);

void slex_add_token_kind(sLex *l, const char *re, const char *type);
void slex_add_source_str(sLex *l, const char *source_str);

void slex_remove_source_str(sLex *l);

sTok slex_get_next_token(sLex *l);
sTok slex_peek_next_token(sLex *l);
int slex_is_complete(sLex *l);

void slex_free(sLex *l);

#endif // SLEX_H_
