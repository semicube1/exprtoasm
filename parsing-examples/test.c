#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../include/slex.h"
#include "../include/sparser.h"

#define MAX_SOURCE_STR_LEN 1024

int main(void)
{
  static char source_str[MAX_SOURCE_STR_LEN + 1];
  sLex *l = slex_new();
  sParser *p = sparser_new();

  FILE *input_fp = fopen("./input.txt", "r");
  fgets(source_str, sizeof(source_str), input_fp);

  if (source_str[strlen(source_str)-1] == '\n')
    source_str[strlen(source_str)-1] = '\0';

  fclose(input_fp);
  printf("[INPUT]\n\t%s", source_str);

  FILE *lex_fp = fopen("./lex.txt", "r");
  char token_kind[128] = {0}, re[128] = {0};
  while (fscanf(lex_fp, "%s %s", token_kind, re) != EOF) {
    slex_add_token_kind(l, re, token_kind);
    sparser_add_terminal(p, token_kind);
  }
  slex_add_source_str(l, source_str);
  fclose(lex_fp);

  FILE *grammar_fp = fopen("./grammar.txt", "r");
  char line_buffer[256] = {0};
  while (fgets(line_buffer, sizeof(line_buffer), grammar_fp) != NULL) {

    if (line_buffer[strlen(line_buffer) - 1] == '\n')
      line_buffer[strlen(line_buffer) - 1] = '\0';

    char *grammar_rule = strtok(line_buffer, ":");
    char *semantic_action = strtok(NULL, ":");

    if (semantic_action != NULL)
      sparser_add_bnf_rule(p, grammar_rule, semantic_action);
    else
      sparser_add_bnf_rule(p, grammar_rule, "");
  }
  fclose(grammar_fp);

  sparser_find_first(p);
  sparser_find_states(p);

  printf("\n[TERMINALS]\n\t");
  sparser_print_terminals(stdout, p);

  printf("\n[NON TERMINALS]\n\t");
  sparser_print_nonterminals(stdout, p);

  printf("\n[AUGMENTED GRAMMAR]\n");
  sparser_print_grammar(p);

  printf("\n[FIRST]\n");
  sparser_print_first(p);

  printf("\n[STATES]\n\n");
  sparser_print_states(p);

  printf("\n[PARSING TABLE]\n\n");
  sparser_print_table(p);

  printf("\n[TOKENS]\n");
  printf("\t{ ");
  while (slex_is_complete(l) != 1) {
    sTok t = slex_get_next_token(l);
    printf("'%s': %s, ", t.lexeme, t.type);
  }
  printf("}\n");

  slex_remove_source_str(l);
  slex_add_source_str(l, source_str);

  printf("\n[STEP-BY-STEP PARSING]\n\n");

  AST *ast = sparser_parse_string(p, l, true);

  printf("\n\n[AST]\n\n");
  sparser_print_ast(ast, 0);

  slex_free(l);
  // sparser_free(p);

  return 0;
}
