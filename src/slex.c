#include "../include/slex.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <regex.h>

sLex *slex_new(void)
{
  sLex *l = malloc(sizeof(*l));
  l->token_kinds_count = 0;
  l->token_kinds_cap = INIT_TOK_KINDS_CAP;
  l->token_kinds = malloc(sizeof(*(l->token_kinds)) * l->token_kinds_cap);

  l->source_str = NULL;
  l->position = 0;

  return l;
}

void slex_add_token_kind(sLex *l, const char *re, const char *type)
{
  // Doubling cap if cap is insufficient
  if (l->token_kinds_count >= l->token_kinds_cap) {
    l->token_kinds_cap = l->token_kinds_cap * 2;
    l->token_kinds = realloc(l->token_kinds, sizeof(*(l->token_kinds)) * l->token_kinds_cap);
  }

  strcpy(l->token_kinds[l->token_kinds_count].type, type);
  l->token_kinds[l->token_kinds_count].re = malloc(sizeof(char) * (strlen(re) + 1));
  strcpy(l->token_kinds[l->token_kinds_count].re, re); 

  l->token_kinds_count++;
}

void slex_add_source_str(sLex *l, const char *source_str)
{
  l->source_str = malloc(sizeof(char) * (strlen(source_str) + 1));
  strcpy(l->source_str, source_str); 

  l->position = 0;
}

void slex_remove_source_str(sLex *l)
{
  free(l->source_str);

  l->source_str = NULL;
  l->position = 0;
}

sTok slex_peek_next_token(sLex *l)
{
  sTok t;

  regex_t regex = {0}; 
  regmatch_t regmatch = {0};

  size_t min_rm_so = strlen(l->source_str) - l->position;
  size_t min_rm_eo = strlen(l->source_str) - l->position;
  char type[LEXEME_MAX_LEN + 1];

  // Left trimming whitespaces
  while (l->position < strlen(l->source_str) && isspace(l->source_str[l->position]) > 0) {
    l->position++;
  }

  if (slex_is_complete(l) == 1) {
    strcpy(t.type, "SLEX_END_TOK_TYPE");
    t.lexeme[0] = '\0';

    return t;
  }

  for (size_t i = 0; i < l->token_kinds_count; ++i) {
    assert(regcomp(&regex, l->token_kinds[i].re, REG_EXTENDED) == 0);

    if (regexec(&regex, l->source_str + l->position, 1, &regmatch, 0) == 0) {
      if (min_rm_so > (size_t) regmatch.rm_so) {
        min_rm_so = (size_t) regmatch.rm_so;
        min_rm_eo = (size_t) regmatch.rm_eo;
        strcpy(type, l->token_kinds[i].type);
      }
    }
  }

  if (min_rm_so == 0) {
    assert(min_rm_eo - min_rm_so < LEXEME_MAX_LEN);
    sprintf(t.lexeme, "%.*s", (int) (min_rm_eo - min_rm_so), l->source_str + l->position);
    strcpy(t.type, type);

    //l->position += min_rm_eo - min_rm_so;
    
    regfree(&regex);
    return t;
  }

  assert(min_rm_so < LEXEME_MAX_LEN);
  sprintf(t.lexeme, "%.*s", (int) min_rm_so ,l->source_str + l->position);
  strcpy(t.type, "SLEX_INVALID_TOK_TYPE");

  //l->position += min_rm_so;

  regfree(&regex);
  return t;
}

sTok slex_get_next_token(sLex *l)
{
  sTok t;

  regex_t regex = {0}; 
  regmatch_t regmatch = {0};

  size_t min_rm_so = strlen(l->source_str) - l->position;
  size_t min_rm_eo = strlen(l->source_str) - l->position;
  char type[LEXEME_MAX_LEN + 1];

  // Left trimming whitespaces
  while (l->position < strlen(l->source_str) && isspace(l->source_str[l->position]) > 0) {
    l->position++;
  }

  if (slex_is_complete(l) == 1) {
    strcpy(t.type, "SLEX_END_TOK_TYPE");
    t.lexeme[0] = '\0';

    return t;
  }

  for (size_t i = 0; i < l->token_kinds_count; ++i) {
    assert(regcomp(&regex, l->token_kinds[i].re, REG_EXTENDED) == 0);

    if (regexec(&regex, l->source_str + l->position, 1, &regmatch, 0) == 0) {
      if (min_rm_so > (size_t) regmatch.rm_so) {
        min_rm_so = (size_t) regmatch.rm_so;
        min_rm_eo = (size_t) regmatch.rm_eo;
        strcpy(type, l->token_kinds[i].type);
      }
    }
  }

  if (min_rm_so == 0) {
    assert(min_rm_eo - min_rm_so < LEXEME_MAX_LEN);
    sprintf(t.lexeme, "%.*s", (int) (min_rm_eo - min_rm_so), l->source_str + l->position);
    strcpy(t.type, type);

    l->position += min_rm_eo - min_rm_so;
    
    regfree(&regex);
    return t;
  }

  assert(min_rm_so < LEXEME_MAX_LEN);
  sprintf(t.lexeme, "%.*s", (int) min_rm_so ,l->source_str + l->position);
  strcpy(t.type, "SLEX_INVALID_TOK_TYPE");

  l->position += min_rm_so;

  regfree(&regex);
  return t;
}


int slex_is_complete(sLex *l)
{
  return (l->position >= strlen(l->source_str));
}

void slex_free(sLex *l)
{
  for (size_t i = 0; i < l->token_kinds_count; ++i)
    free(l->token_kinds[i].re);
  free(l->token_kinds);
  free(l->source_str);
  free(l);
}
