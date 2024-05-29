#ifndef SPARSER_H_
#define SPARSER_H_

#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>

#include "./slex.h"

typedef struct {
  char label[64];
} _Terminals;

typedef struct {
  char label[64];

  char *first[64];
  size_t first_count;
} _NonTerminals;

typedef struct {
  char LHS[16];
  struct {
    char label[16];
    int isNonTerminal;
  } RHS[16];
  size_t RHS_count;

  int semantic_rule_type;
  int root_node_pos;
  int stack_positions_to_pop[10];
  size_t stack_positions_count;
} _Grammar;

typedef struct {
  struct {
    size_t grammar_id;
    size_t dotposition;
    char *lookahead[16];
    size_t lookahead_count;
  } dotted_grammar[32];
  size_t dotted_grammar_count;

  int goto_table[32];
  struct {
    struct {
      int type; // -1: None, 0: Shift, 1:Reduce
      int number;
    } actions[8];
    size_t action_count;
  } action_table[32];
} _State;

typedef struct {
  _Terminals *terminals[64];
  size_t terminals_count;

  _NonTerminals *non_terminals[64];
  size_t non_terminals_count;

  _Grammar *grammars[64];
  size_t grammar_count;

  _State *states[64];
  size_t state_count;
} sParser;


typedef struct _AST {
  char text[128];
  size_t no_of_children;
  struct _AST *child[10];
} AST;

sParser *sparser_new(void);

void sparser_add_terminal(sParser *p, const char *terminal);
void sparser_print_terminals(FILE *stream, sParser *p);
void sparser_print_nonterminals(FILE *stream, sParser *p);
void sparser_print_grammar(sParser *p);
void sparser_add_bnf_rule(sParser *p, const char *rule, const char *semantic_rule);
void sparser_find_first_of(sParser *p, size_t non_terminal_id);

void sparser_find_first(sParser *p);

void sparser_state_closure_debug(sParser *p, _State *s);
void sparser_state_closure(sParser *p, _State *s);

void sparser_print_states(sParser *q);

int sparser_compare_states(_State *s1, _State *s2);

void sparser_find_states(sParser *p);

void sparser_print_first(sParser *p);

void sparser_print_table(sParser *p);

AST *sparser_parse_string(sParser *p, sLex *l, bool verbose_flag);

void sparser_print_ast(AST *ast, size_t level);

#endif // SPARSER_H_
