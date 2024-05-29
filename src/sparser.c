#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/sparser.h"
#include "../include/slex.h"

sParser *sparser_new(void)
{
  sParser *p = malloc(sizeof(*p));

  p->terminals_count = 0;
  p->non_terminals_count = 0;
  p->grammar_count = 0;
  p->state_count = 0;

  return p;
}

void sparser_add_terminal(sParser *p, const char *terminal)
{
  p->terminals[p->terminals_count] = malloc(sizeof(*(p->terminals[0])));
  strcpy(p->terminals[p->terminals_count]->label, terminal);

  p->terminals_count++;
}

void sparser_print_terminals(FILE *stream, sParser *p)
{
  for (size_t i = 0; i < p->terminals_count; ++i) {
    fprintf(stream, "%s ", p->terminals[i]->label);
  }
  fprintf(stream, "\n");
}

void sparser_print_nonterminals(FILE *stream, sParser *p)
{
  for (size_t i = 0; i < p->non_terminals_count; ++i) {
    fprintf(stream, "%s ", p->non_terminals[i]->label);
  }
  fprintf(stream, "\n");
}

void sparser_print_grammar(sParser *p)
{
  for (size_t i = 0; i < p->grammar_count; ++i) {
    printf("\t(%zu) %s -> ", i, p->grammars[i]->LHS);
    for (size_t j = 0; j < p->grammars[i]->RHS_count; ++j) {
      //printf("[%s, %d] ", p->grammars[i]->RHS[j].label,
      //                   p->grammars[i]->RHS[j].isNonTerminal);
      printf("%s ", p->grammars[i]->RHS[j].label);
    }
    printf("\n");
  }
}

void sparser_add_bnf_rule(sParser *p, const char *rule, const char *semantic_rule)
{
  static int firstRule = 1;

  p->grammars[p->grammar_count] = malloc(sizeof(*(p->grammars[0])));
  p->grammars[p->grammar_count]->RHS_count = 0;

  char tmp[64] = {0};
  strcpy(tmp, rule);

  char *token = strtok(tmp, " ");

  if (firstRule == 1) {
    firstRule = 0;

    strcpy(p->grammars[p->grammar_count]->LHS, "S'");
    size_t RHS_count = p->grammars[p->grammar_count]->RHS_count;
    strcpy(p->grammars[p->grammar_count]->RHS[RHS_count].label, token);

    p->grammars[p->grammar_count]->RHS[RHS_count].isNonTerminal = 1;

    p->grammars[p->grammar_count]->RHS_count++;
    p->grammar_count++;

    p->grammars[p->grammar_count] = malloc(sizeof(*(p->grammars[0])));
    p->grammars[p->grammar_count]->RHS_count = 0;

    p->non_terminals[p->non_terminals_count] = malloc(sizeof(*(p->non_terminals[0])));
    strcpy(p->non_terminals[p->non_terminals_count]->label, "S'");


    p->non_terminals[p->non_terminals_count]->first_count = 0;
    p->non_terminals_count++;
  }

  strcpy(p->grammars[p->grammar_count]->LHS, token);

  {
    int isNonTerminalAlreadyPresent = 0;
    for (size_t i = 0; i < p->non_terminals_count; ++i) {
      if (strcmp(p->non_terminals[i]->label, token) == 0) {
        isNonTerminalAlreadyPresent = 1;
        break;
      }
    }
    if (isNonTerminalAlreadyPresent == 0) {
      p->non_terminals[p->non_terminals_count] = malloc(sizeof(*(p->non_terminals[p->non_terminals_count])));
      strcpy(p->non_terminals[p->non_terminals_count]->label, token);

      p->non_terminals[p->non_terminals_count]->first_count = 0;
      p->non_terminals_count++;
    }
  }

  token = strtok(NULL, " ");
  token = strtok(NULL, " ");

  while (token != NULL) {
    size_t RHS_count = p->grammars[p->grammar_count]->RHS_count;

    strcpy(p->grammars[p->grammar_count]->RHS[RHS_count].label, token);
    p->grammars[p->grammar_count]->RHS[RHS_count].isNonTerminal = 1;

    int isTerminalflag = 0;
    for (size_t i = 0; i < p->terminals_count; ++i) {
      if (strcmp(p->terminals[i]->label, token) == 0) {
        isTerminalflag = 1;
        p->grammars[p->grammar_count]->RHS[RHS_count].isNonTerminal = 0;
        break;
      }
    }

    if (isTerminalflag != 1) {
      int isNonTerminalAlreadyPresent = 0;
      for (size_t i = 0; i < p->non_terminals_count; ++i) {
        if (strcmp(p->non_terminals[i]->label, token) == 0) {
          isNonTerminalAlreadyPresent = 1;
          break;
        }
      }
      if (isNonTerminalAlreadyPresent == 0) {
        p->non_terminals[p->non_terminals_count] = malloc(sizeof(*(p->non_terminals[p->non_terminals_count])));
        strcpy(p->non_terminals[p->non_terminals_count]->label, token);

        p->non_terminals[p->non_terminals_count]->first_count = 0;
        p->non_terminals_count++;
      }
    }

    p->grammars[p->grammar_count]->RHS_count++;

    token = strtok(NULL, " ");
  }

  // Adding semantic rule thingy
  //strcpy(p->grammars[p->grammar_count]->semantic_rule, semantic_rule);

  char tmp2[64];
  strcpy(tmp2, semantic_rule);

  char *tok2 = strtok(tmp2, " ");

  /*
  while (tok2 != NULL) {
    printf("%s ", tok2);
    tok2 = strtok(NULL, " ");
  }
  if (tok2 == NULL) goto here;
  if (strcmp(tok2, "node") == 0) {
    printf("<NODE>: ");
  } else if (strcmp(tok2, "leaf") == 0) {
    printf("<LEAF>: ");
  } else {
    printf("<NONE>: ");
  }
  here:
  printf("\n")kkk
  */
  if (tok2 == NULL) {
    p->grammars[p->grammar_count]->semantic_rule_type = -1;
    p->grammars[p->grammar_count]->stack_positions_count = 0;

    p->grammar_count++;
    return;
  }
  
  if (strcmp(tok2, "node") == 0) {
    p->grammars[p->grammar_count]->semantic_rule_type = 0; // NODE

    tok2 = strtok(NULL, " ");

    p->grammars[p->grammar_count]->root_node_pos = atoi(tok2);

    size_t r = 0;
    p->grammars[p->grammar_count]->stack_positions_count = 0;

    while ((tok2 = strtok(NULL, " ")) != NULL) {
      p->grammars[p->grammar_count]->stack_positions_to_pop[r] = atoi(tok2);
      p->grammars[p->grammar_count]->stack_positions_count++;
      r++;
    }

  } 
  else if (strcmp(tok2, "leaf") == 0) {
    p->grammars[p->grammar_count]->semantic_rule_type = 1; // LEAF

    tok2 = strtok(NULL, " ");

    p->grammars[p->grammar_count]->root_node_pos = atoi(tok2);
    p->grammars[p->grammar_count]->stack_positions_count = 0;
  } 

  p->grammar_count++;
}

void sparser_print_first(sParser *p)
{
  for (size_t i = 0; i < p->non_terminals_count; ++i) {
    printf("\tFIRST(%s) = { ", p->non_terminals[i]->label);
    for (size_t j = 0; j < p->non_terminals[i]->first_count; ++j) {
      printf("%s, ", p->non_terminals[i]->first[j]);
    }
    printf("}\n");
  }
}

void sparser_find_first_of(sParser *p, size_t non_terminal_id)
{
  size_t i = non_terminal_id;
  
  for (size_t j = 0; j < p->grammar_count; ++j) {
    if (strcmp(p->grammars[j]->LHS, p->non_terminals[i]->label) == 0) {
      if (strcmp(p->grammars[j]->RHS[0].label, p->non_terminals[i]->label) == 0) {
        continue;
      }
      else if (p->grammars[j]->RHS[0].isNonTerminal == 0) {
        int isAlreadyPresentinFirst = 0;
        for (size_t k = 0; k < p->non_terminals[i]->first_count; ++k) {
            if (strcmp(p->non_terminals[i]->first[k], p->grammars[j]->RHS[0].label) == 0) {
              isAlreadyPresentinFirst = 1;
              break;
            }
        }

        // if (non_terminal_id == 3)
        //   printf("Reached here!!, for F\n");

        if (isAlreadyPresentinFirst == 1) continue;
        // printf("Not alrady pre\n");

        size_t first_count = p->non_terminals[i]->first_count;
        p->non_terminals[i]->first[first_count] = malloc(32);
        strcpy(p->non_terminals[i]->first[first_count], p->grammars[j]->RHS[0].label);
        p->non_terminals[i]->first_count++;
      }
      else {
        size_t this_non_terminal_id = 999;

        for (size_t k = 0; k < p->non_terminals_count; ++k) {
          if (strcmp(p->non_terminals[k]->label, p->grammars[j]->RHS[0].label) == 0) {
            this_non_terminal_id = k;
            break;
          }
        }
        assert(this_non_terminal_id != 999);

        // if (non_terminal_id == 2)
        //    printf("WE REACHED HERE WITH, next_non_term id = %zu\n", this_non_terminal_id);

        sparser_find_first_of(p, this_non_terminal_id);

        // if (non_terminal_id == 2)
        //    printf("(we also completed the search) next_non_term id = %zu\n", this_non_terminal_id);

        for (size_t k = 0; k < p->non_terminals[this_non_terminal_id]->first_count; ++k) {
          int isalreadypresent = 0;
          for (size_t l = 0; l < p->non_terminals[i]->first_count; ++l) {
            if (strcmp(p->non_terminals[i]->first[l], p->non_terminals[this_non_terminal_id]->first[k]) == 0) {
              isalreadypresent = 1;
              break;
            }
          }
          // printf("Is already present??? --> %d\n", isalreadypresent);
          if (isalreadypresent == 1) continue;

          size_t first_count = p->non_terminals[i]->first_count;
          p->non_terminals[i]->first[first_count] = malloc(32);
          strcpy(p->non_terminals[i]->first[first_count], p->non_terminals[this_non_terminal_id]->first[k]);
          p->non_terminals[i]->first_count++;
        }
      }
    }
  }
}

void sparser_state_closure(sParser *p, _State *s)
{
  for (size_t i = 0; i < s->dotted_grammar_count; ++i) {
  // for (size_t i = 0; i < 2; ++i) {
    // size_t grammar_id = s->dotted_grammar[i].grammar_id;

    // We need to find the lookaheads
    char *lookaheads[32];
    size_t lookaheads_found_so_far = 0;

    if (s->dotted_grammar[i].dotposition == p->grammars[s->dotted_grammar[i].grammar_id]->RHS_count) {
      // If dot is at the very end
      continue;
    }
    else if (p->grammars[s->dotted_grammar[i].grammar_id]->RHS[s->dotted_grammar[i].dotposition].isNonTerminal == 0) {
      // if a terminal immediately follows the dot
      continue;
    }
    else if (s->dotted_grammar[i].dotposition + 1 >= p->grammars[s->dotted_grammar[i].grammar_id]->RHS_count) {
      for (size_t j = 0; j < s->dotted_grammar[i].lookahead_count; ++j) {
        lookaheads[j] = malloc(32);
        strcpy(lookaheads[j], s->dotted_grammar[i].lookahead[j]);
        lookaheads_found_so_far++;
      }
      // DEBUGGING
      /*
      if (i == 0) {
        printf("closure --> Got here!! LOOKAHEADS ->\n\t");
        for (size_t r = 0; r < lookaheads_found_so_far; ++r) {
          printf("%s, ", lookaheads[r]);
        }
        printf("\n");
      }
      */
    }
    else if (p->grammars[s->dotted_grammar[i].grammar_id]->RHS[s->dotted_grammar[i].dotposition + 1].isNonTerminal == 0)
    {
      lookaheads[0] = malloc(32);
      strcpy(lookaheads[0], p->grammars[s->dotted_grammar[i].grammar_id]->RHS[s->dotted_grammar[i].dotposition+1].label);
      lookaheads_found_so_far++;
    }
    else if (p->grammars[s->dotted_grammar[i].grammar_id]->RHS[s->dotted_grammar[i].dotposition + 1].isNonTerminal == 1)
    {
      for (size_t j = 0; j < p->non_terminals_count; ++j) {
        if (strcmp(p->non_terminals[j]->label, p->grammars[s->dotted_grammar[i].grammar_id]->RHS[s->dotted_grammar[i].dotposition + 1].label) == 0) {
          for (size_t k = 0; k < p->non_terminals[j]->first_count; ++k) {
            lookaheads[k] = malloc(32);
            strcpy(lookaheads[k], p->non_terminals[j]->first[k]);
            lookaheads_found_so_far++;
          }
          break;
        }
      }
    }

    for (size_t g = 0; g < p->grammar_count; ++g) {
      if (strcmp(p->grammars[s->dotted_grammar[i].grammar_id]->RHS[s->dotted_grammar[i].dotposition].label, p->grammars[g]->LHS) == 0) {
        int is_grammar_rule_present = 0;

        for (size_t h = 0; h < s->dotted_grammar_count; ++h) {
          if (s->dotted_grammar[h].grammar_id == g && s->dotted_grammar[h].dotposition == 0) {
            is_grammar_rule_present = 1;
            for (size_t x = 0; x < lookaheads_found_so_far; ++x) {
              int is_it_there = 0;
              for (size_t y = 0; y < s->dotted_grammar[h].lookahead_count; ++y) {
                if (strcmp(s->dotted_grammar[h].lookahead[y], lookaheads[x]) == 0) {
                  is_it_there = 1;
                  break;
                }
              }
              if (is_it_there == 0) {
                // printf("[is_it_there] It's not there lmfaoo\n");
                s->dotted_grammar[h].lookahead[s->dotted_grammar[h].lookahead_count] = malloc(32);
                strcpy(s->dotted_grammar[h].lookahead[s->dotted_grammar[h].lookahead_count], lookaheads[x]);
                s->dotted_grammar[h].lookahead_count++;
              }
            }
          }
        }

        if (is_grammar_rule_present == 0) {
          // printf("[is_grammar_rule_present] It's not there lmfaoo\n");
          s->dotted_grammar[s->dotted_grammar_count].grammar_id = g;
          s->dotted_grammar[s->dotted_grammar_count].dotposition = 0;
          s->dotted_grammar[s->dotted_grammar_count].lookahead_count = 0;

          for (size_t a = 0; a < lookaheads_found_so_far; ++a) {
            s->dotted_grammar[s->dotted_grammar_count].lookahead[a] = malloc(32);
            strcpy(s->dotted_grammar[s->dotted_grammar_count].lookahead[a], lookaheads[a]);
            s->dotted_grammar[s->dotted_grammar_count].lookahead_count++;
          }
          s->dotted_grammar_count++;
        }
      }
    }

    for (size_t m = 0; m < lookaheads_found_so_far; ++m)
      free(lookaheads[m]);
  }
}

void sparser_print_states(sParser *p)
{
  for (size_t i = 0; i < p->state_count; ++i) {
    printf("State (%zu):\n", i);
    for (size_t j = 0; j < p->states[i]->dotted_grammar_count; ++j) {
      size_t grammar_id = p->states[i]->dotted_grammar[j].grammar_id;
      printf("\t%s -> ", p->grammars[grammar_id]->LHS);

      for (size_t k = 0; k < p->grammars[grammar_id]->RHS_count; ++k) {
        if (p->states[i]->dotted_grammar[j].dotposition == k) {
          printf(". ");
        }
        printf("%s ", p->grammars[grammar_id]->RHS[k].label);
      }
      if (p->states[i]->dotted_grammar[j].dotposition == p->grammars[grammar_id]->RHS_count) {
        printf(". ");
      }

      printf("\t| ");
      for (size_t k = 0; k < p->states[i]->dotted_grammar[j].lookahead_count; ++k) {
        printf("%s, ", p->states[i]->dotted_grammar[j].lookahead[k]);
      }
      printf("\n");
    }

    //printf("\n\tGOTO LIST := ");
    int is_there_any_goto = 0;
    for (size_t j = 0; j < p->non_terminals_count; ++j) {
      if (p->states[i]->goto_table[j] != -1) {
        if (is_there_any_goto == 0) {
          is_there_any_goto = 1;
          printf("\n");
        }
        printf("\tGOTO(%zu, %s) = %d\n", i, p->non_terminals[j]->label, p->states[i]->goto_table[j]);
      }
      //printf("%d, ", p->states[i]->goto_table[j]);
    }
    //printf("\n");
    int is_there_any_action = 0;
    //printf("\n\tAction LIST := ");
    for (size_t j = 0; j < p->terminals_count; ++j) {
      if (p->states[i]->action_table[j].action_count > 0) {
        if (is_there_any_action == 0) {
          is_there_any_action = 1;
          printf("\n");
        }
        printf("\tACTION(%zu, %s) = ", i, p->terminals[j]->label);
        for (size_t k = 0; k < p->states[i]->action_table[j].action_count; ++k) {
          if (p->states[i]->action_table[j].actions[k].type == 0) {
            printf("S");
          } else if (p->states[i]->action_table[j].actions[k].type == 1) {
            printf("R");
          }
          printf("%d", p->states[i]->action_table[j].actions[k].number);
          if (k < p->states[i]->action_table[j].action_count - 1)
            printf(" | ");
        }
        printf("\n");
      }
    }
    // Seperate case for '$'

    if (p->states[i]->action_table[p->terminals_count].action_count > 0) {
      for (size_t k = 0; k < p->states[i]->action_table[p->terminals_count].action_count; ++k) {
        if (is_there_any_action == 0) {
          is_there_any_action = 1;
          printf("\n");
        }
        printf("\tACTION(%zu, $) = ", i);
        if (p->states[i]->action_table[p->terminals_count].actions[k].type == 0) {
          printf("S");
        } else if (p->states[i]->action_table[p->terminals_count].actions[k].type == 1) {
          printf("R");
        }
        printf("%d", p->states[i]->action_table[p->terminals_count].actions[k].number);
        if (k < p->states[i]->action_table[p->terminals_count].action_count - 1)
          printf(" | ");
      }
      printf("\n");
    }

    printf("\n");
  }
}
int sparser_compare_states(_State *s1, _State *s2)
{
  if (s1->dotted_grammar_count != s2->dotted_grammar_count) return -1;
  // int is_all_grammar_present = 1;

  for (size_t i = 0; i < s1->dotted_grammar_count; ++i) {
    int did_we_match_this_grammarrule_anywhere = 0;
    
    for (size_t j = 0; j < s2->dotted_grammar_count; ++j) {
      if (s1->dotted_grammar[i].dotposition == s2->dotted_grammar[j].dotposition &&
          s1->dotted_grammar[i].grammar_id == s2->dotted_grammar[j].grammar_id) {
            
            if (s1->dotted_grammar[i].lookahead_count != s2->dotted_grammar[j].lookahead_count) 
              continue;
            
            int all_lookaheads_present = 1;
            for (size_t k = 0; k < s1->dotted_grammar[i].lookahead_count; ++k) {
              int did_we_match_this_lookaheadsymbol_anywhere = 0;
              for (size_t l = 0; l < s2->dotted_grammar[j].lookahead_count; ++l) {
                if (strcmp(s1->dotted_grammar[i].lookahead[k], s2->dotted_grammar[j].lookahead[l]) == 0) {
                  did_we_match_this_lookaheadsymbol_anywhere = 1;
                  break;
                }
              }
              if (did_we_match_this_lookaheadsymbol_anywhere == 0) {
                all_lookaheads_present = 0;
                break;
              }
            }

            if (all_lookaheads_present == 1) {
              did_we_match_this_grammarrule_anywhere = 1;
              break;
            }
      }
    }

    if (did_we_match_this_grammarrule_anywhere == 0) {
      return -1;
    }
  }
  return 0;
}

void sparser_find_states(sParser *p)
{
  p->states[p->state_count] = malloc(sizeof(*(p->states[0])));

  p->states[p->state_count]->dotted_grammar_count = 0;
  p->states[p->state_count]->dotted_grammar[0].grammar_id = 0;
  p->states[p->state_count]->dotted_grammar[0].dotposition = 0;
  p->states[p->state_count]->dotted_grammar[0].lookahead_count = 0;

  p->states[p->state_count]->dotted_grammar[0].lookahead[0] = malloc(32);
  strcpy(p->states[p->state_count]->dotted_grammar[0].lookahead[0], "$");
  p->states[p->state_count]->dotted_grammar[0].lookahead_count++;
  p->states[p->state_count]->dotted_grammar_count++;

  p->state_count++;

  for (size_t i = 0; i < p->state_count; ++i) {
    //sparser_state_closure(p, p->states[p->state_count]);
    sparser_state_closure(p, p->states[i]);

    // Goto part

    for (size_t j = 0; j < p->non_terminals_count; ++j) {

      _State *s = malloc(sizeof(*s));
      s->dotted_grammar_count = 0;

      int do_we_need_this_transition = 0;

      for (size_t k = 0; k < p->states[i]->dotted_grammar_count; ++k) {
        size_t pos = p->states[i]->dotted_grammar[k].dotposition;
        size_t grammar_id = p->states[i]->dotted_grammar[k].grammar_id;
        if (strcmp(p->grammars[grammar_id]->RHS[pos].label, p->non_terminals[j]->label) == 0) {
          // Add the rule with dotposition + 1 to the new state
          do_we_need_this_transition = 1;

          s->dotted_grammar[s->dotted_grammar_count].grammar_id = grammar_id;
          s->dotted_grammar[s->dotted_grammar_count].dotposition = pos + 1;

          s->dotted_grammar[s->dotted_grammar_count].lookahead_count = 0;
          for (size_t l = 0; l < p->states[i]->dotted_grammar[k].lookahead_count; ++l) {
            s->dotted_grammar[s->dotted_grammar_count].lookahead[l] = malloc(32);
            strcpy(s->dotted_grammar[s->dotted_grammar_count].lookahead[l], p->states[i]->dotted_grammar[k].lookahead[l]);
            s->dotted_grammar[s->dotted_grammar_count].lookahead_count++;
          }

          s->dotted_grammar_count++;
        }
      }

      if (do_we_need_this_transition == 0) {
        free(s);
        p->states[i]->goto_table[j] = -1;
        continue;
      }

      // So I got the new state with all of the needed grammar rules with updated dot + 1
      sparser_state_closure(p, s);

      int does_state_already_exist = 0;

      for (size_t k = 0; k < p->state_count; ++k) {
        if (sparser_compare_states(p->states[k], s) == 0) {
          does_state_already_exist = 1;
          
          // Freeing memory
          for (size_t l = 0; l < s->dotted_grammar_count; ++l) {
            for (size_t m = 0; m < s->dotted_grammar[l].lookahead_count; ++m) {
              free(s->dotted_grammar[l].lookahead[m]);
            }
          }
          free(s);

          p->states[i]->goto_table[j] = k;
          break;
        }
      }

      if (does_state_already_exist == 0) {
        p->states[p->state_count] = s;
        p->state_count++;
        p->states[i]->goto_table[j] = p->state_count - 1;
      }
    }

    // Shift Part
    for (size_t j = 0; j < p->terminals_count; ++j) {
      _State *s = malloc(sizeof(*s));
      s->dotted_grammar_count = 0;
      p->states[i]->action_table[j].action_count = 0;

      int do_we_need_this_transition = 0;

      for (size_t k = 0; k < p->states[i]->dotted_grammar_count; ++k) {
        size_t pos = p->states[i]->dotted_grammar[k].dotposition;
        size_t grammar_id = p->states[i]->dotted_grammar[k].grammar_id;
        if (strcmp(p->grammars[grammar_id]->RHS[pos].label, p->terminals[j]->label) == 0) {
          // Add the rule with dotposition + 1 to the new state
          do_we_need_this_transition = 1;

          s->dotted_grammar[s->dotted_grammar_count].grammar_id = grammar_id;
          s->dotted_grammar[s->dotted_grammar_count].dotposition = pos + 1;

          s->dotted_grammar[s->dotted_grammar_count].lookahead_count = 0;
          for (size_t l = 0; l < p->states[i]->dotted_grammar[k].lookahead_count; ++l) {
            s->dotted_grammar[s->dotted_grammar_count].lookahead[l] = malloc(32);
            strcpy(s->dotted_grammar[s->dotted_grammar_count].lookahead[l], p->states[i]->dotted_grammar[k].lookahead[l]);
            s->dotted_grammar[s->dotted_grammar_count].lookahead_count++;
          }

          s->dotted_grammar_count++;
        }
      }

      if (do_we_need_this_transition == 0) {
        free(s);
        // Actually we can add the "-1" thingy after we confirm there's no reduce action either
        // Adding this for the time being
        /*
        size_t action_count = p->states[i]->action_table[j].action_count;
        p->states[i]->action_table[j].actions[action_count].type = -1;
        p->states[i]->action_table[j].actions[action_count].number = -1;
        p->states[i]->action_table[j].action_count++;
        */
        continue;
      }

      sparser_state_closure(p, s);

      int does_state_already_exist = 0;

      for (size_t k = 0; k < p->state_count; ++k) {
        if (sparser_compare_states(p->states[k], s) == 0) {
          does_state_already_exist = 1;
          
          // Freeing memory
          for (size_t l = 0; l < s->dotted_grammar_count; ++l) {
            for (size_t m = 0; m < s->dotted_grammar[l].lookahead_count; ++m) {
              free(s->dotted_grammar[l].lookahead[m]);
            }
          }
          free(s);

          size_t action_count = p->states[i]->action_table[j].action_count;
          p->states[i]->action_table[j].actions[action_count].type = 0;
          p->states[i]->action_table[j].actions[action_count].number = k;
          p->states[i]->action_table[j].action_count++;
          break;
        }
      }

      if (does_state_already_exist == 0) {
        size_t action_count = p->states[i]->action_table[j].action_count;
        p->states[i]->action_table[j].actions[action_count].type = 0;
        
        p->states[p->state_count] = s;
        p->state_count++;

        p->states[i]->action_table[j].actions[action_count].number = p->state_count - 1;
        p->states[i]->action_table[j].action_count++;
        // ---
      }
    }

    // Adding the '$' in Action part
    // aka the REDUCE part

    for (size_t j = 0; j < p->states[i]->dotted_grammar_count; ++j) {
      size_t grammar_id = p->states[i]->dotted_grammar[j].grammar_id;
      if (p->states[i]->dotted_grammar[j].dotposition == p->grammars[grammar_id]->RHS_count) {
        for (size_t k = 0; k < p->states[i]->dotted_grammar[j].lookahead_count; ++k) {
          if (strcmp(p->states[i]->dotted_grammar[j].lookahead[k], "$") == 0) {
            size_t action_count = p->states[i]->action_table[p->terminals_count].action_count;
            p->states[i]->action_table[p->terminals_count].actions[action_count].type = 1;
            p->states[i]->action_table[p->terminals_count].actions[action_count].number = grammar_id;
            p->states[i]->action_table[p->terminals_count].action_count++;

            // if (i == 2) {
              // printf("NEW NEW NEW [[[State %zu]]], action_count of '$' = %zu\n", i, p->states[i]->action_table[p->terminals_count].action_count);
            // }
            continue;
          }
          // else if, we need to find appropriate terminal ids for the things in the lookahead

          size_t terminal_id = 999;
          for (size_t l = 0; l < p->terminals_count; ++l) {
            if (strcmp(p->terminals[l]->label, p->states[i]->dotted_grammar[j].lookahead[k]) == 0) {
              terminal_id = l;
              break;
            }
          }
          // printf("State = %zu, Terminal_id value = %zu\n", i, terminal_id);
          assert(terminal_id != 999);

          size_t action_count = p->states[i]->action_table[terminal_id].action_count;
          p->states[i]->action_table[terminal_id].actions[action_count].type = 1;
          p->states[i]->action_table[terminal_id].actions[action_count].number = grammar_id;
          p->states[i]->action_table[terminal_id].action_count++;
        }
      }
    }

    /*
    printf("(Debugging Action) State %zu --> (count) ", i);
    for (size_t u = 0; u <= p->terminals_count; ++u) {
      printf("%zu ", p->states[i]->action_table[u].action_count);
    }
    printf("\n");
    */
  }
}

void sparser_find_first(sParser *p)
{
  for (size_t i = 0; i < p->non_terminals_count; ++i) {
    sparser_find_first_of(p, i);
  }
}

AST *sparser_parse_string(sParser *p, sLex *l, bool verbose_flag)
{
  char parser_stack[32][32] = {0};
  size_t parser_stack_top = 0;

  char parser_text[32][64] = {0};
  size_t parser_text_top = 0;

  AST *ast_stack[128] = {0};
  size_t ast_stack_top = 0;

  strcpy(parser_stack[0], "0");
  parser_stack_top++;

  size_t step = 1;

  if (verbose_flag == true) printf("Step(s)\t|%-60s|%-40s|Action\t|\n\n", "Stack", "Input");

  //while (slex_is_complete(l) != 1) {
  while (1) {
    if (verbose_flag == true) {
      printf("%zu\t|", step);

      char stack_string[512] = {0};
      for (size_t i = 0; i < parser_stack_top; ++i) {
        sprintf(stack_string + strlen(stack_string), "%s ", parser_stack[i]);
      }
      printf("%-60s|", stack_string);

      char input_string[128] = {0};
      sprintf(input_string, "%s$", l->source_str + l->position);
      printf("%-40s|", input_string);
    }

    int current_state = atoi(parser_stack[parser_stack_top-1]);

    sTok t = slex_peek_next_token(l);

    size_t terminal_id = 999;

    if (strcmp(t.type, "SLEX_INVALID_TOK_TYPE") == 0) {
      if (verbose_flag == true) printf("Error!");

      // ACtually lets just return null here
      return NULL;
    } 
    else if (strcmp(t.type, "SLEX_END_TOK_TYPE") == 0) {
      terminal_id = p->terminals_count;
    } 
    else {
      for (size_t i = 0; i < p->terminals_count; ++i) {
        // TODO: PROBLEM HERE THE LEXEME DOENS"T MATCH THE LABEL OF COURSE
        // FIXED IT
        if (strcmp(p->terminals[i]->label, t.type) == 0) {
          terminal_id = i;
          break;
        }
      }
    }

    //printf("(stacktop = %s, wterminal_id got = %zu)", terminal_id);
    assert(terminal_id != 999);

    // Go with the first action even if there's a conflict (TODO: Deal with conflicts)
    if (p->states[current_state]->action_table[terminal_id].action_count == 0) {
      if (verbose_flag == true) printf("Error\n");
      return NULL;
    }
    if (p->states[current_state]->action_table[terminal_id].actions[0].type == 0) { // Shift
      // LATEST CHANGE (do we need this assert, why did i add this?)
      // if (terminal_id == p->terminals_count) {
      //   printf("current_state = %zu, terminal_id = %zu, terminal = %s")
      //}
      assert(terminal_id != p->terminals_count);

      strcpy(parser_stack[parser_stack_top], p->terminals[terminal_id]->label);
      parser_stack_top++;

      strcpy(parser_text[parser_text_top], t.lexeme);
      parser_text_top++;

      sprintf(parser_stack[parser_stack_top], "%d", p->states[current_state]->action_table[terminal_id].actions[0].number);
      parser_stack_top++;

      if (verbose_flag == true) printf("s%s\t|\n", parser_stack[parser_stack_top - 1]);

      /*
      for (size_t u = 0; u < parser_text_top; ++u)
        printf("%s ", parser_text[u]);
      */

      t = slex_get_next_token(l);
    }

    else if (p->states[current_state]->action_table[terminal_id].actions[0].type == 1) { // Reduce
      //printf("(REDUCTION!)");
      size_t grammar_id = p->states[current_state]->action_table[terminal_id].actions[0].number;

      if (grammar_id == 0) {
        if (verbose_flag == true) printf("Accept\t|\n");
        // TODO: Make sure to return the AST
        /*
        if (ast_stack_top == 0) {
          printf("YO STACK TOP IS ACTUALLY STILL 0\n");
        } else {
          if (ast_stack[ast_stack_top-1] == NULL) {
            printf("AYO STACK TOP IS NULL ( for some reason)\n");
          }
          sparser_print_ast(ast_stack[ast_stack_top-1], 0);
        }
        */
        if (ast_stack_top == 0) return NULL;
        return ast_stack[ast_stack_top-1];
      }
      // Perform your semantic actions here (before popping the items in parse stack)

      //printf("semantic_rule_type = %d\n", p->grammars[grammar_id]->semantic_rule_type);
      if (p->grammars[grammar_id]->semantic_rule_type == 0)  {// NODE 
        AST *ast_node = malloc(sizeof(*ast_node));

        // printf("\nReached node if statement\n");
        int which_node = p->grammars[grammar_id]->root_node_pos;
        strcpy(ast_node->text, parser_text[parser_text_top - which_node]);
        // printf("node if statement: which_node = %d, grammar_id = %zu, text = %s\n", which_node, grammar_id, ast_node->text);

        ast_node->no_of_children = 0;
        for (size_t e = 0; e < p->grammars[grammar_id]->stack_positions_count; ++e) {
          ast_node->child[e] = ast_stack[ast_stack_top - p->grammars[grammar_id]->stack_positions_to_pop[e]];
          ast_node->no_of_children++;
        }
        //ast_node->no_of_children = p->grammars[grammar_id]->stack_positions_count;
        ast_stack_top -= p->grammars[grammar_id]->stack_positions_count;
        ast_stack[ast_stack_top] = ast_node;
        ast_stack_top++;
        
        //strcpy(ast_node->text, parser_text[parser_text_top-1]);

      }
      else if (p->grammars[grammar_id]->semantic_rule_type == 1) { // LEAF
        // printf("\nReached leaf if statement\n");
        AST *ast_node = malloc(sizeof(*ast_node));

        int which_node = p->grammars[grammar_id]->root_node_pos;
        strcpy(ast_node->text, parser_text[parser_text_top - which_node]);
        ast_node->no_of_children = 0;

        // printf("leaf if statement: which_node = %d, grammar_id = %zu = %s\n", which_node, grammar_id, ast_node->text);
        ast_stack[ast_stack_top] = ast_node;
        ast_stack_top++;
      }
      // *** semantic actions end ***

      size_t double_RHS_count = 2 * (p->grammars[grammar_id]->RHS_count);

      parser_stack_top -= double_RHS_count;

      // so this do as well
      parser_text_top -= (p->grammars[grammar_id]->RHS_count);

      strcpy(parser_stack[parser_stack_top], p->grammars[grammar_id]->LHS);
      parser_stack_top++;

      strcpy(parser_text[parser_text_top], p->grammars[grammar_id]->LHS);
      parser_text_top++;

      size_t non_terminal_id = 999;

      for (size_t j = 0; j < p->non_terminals_count; ++j) {
        if (strcmp(p->non_terminals[j]->label, parser_stack[parser_stack_top-1]) == 0) {
          non_terminal_id = j;
          break;
        }
      }

      assert(non_terminal_id != 909);

      current_state = atoi(parser_stack[parser_stack_top - 2]);

      if (p->states[current_state]->goto_table[non_terminal_id] == -1) {
        if (verbose_flag == true) printf("Error\n");
        //if (ast_stack_top == 0) return NULL
        // Actually lets just return NULL here
        return NULL;
      }

      sprintf(parser_stack[parser_stack_top], "%d", p->states[current_state]->goto_table[non_terminal_id]);
      parser_stack_top++;

      if (verbose_flag == true) printf("r%zu\t|\n", grammar_id);

      /*
      for (size_t u = 0; u < parser_text_top; ++u)
        printf("%s ", parser_text[u]);
      */

      /*
      if (ast_stack_top > 0) {
        //printf("\nAST (semantic type = %d): \n", p->grammars[grammar_id]->semantic_rule_type);
        printf("\n{ (ast_stack_top = %zu)\n", ast_stack_top);
        //printf("%s", ast_stack[ast_stack_top-1]->text);
        sparser_print_ast(ast_stack[ast_stack_top-1], 0);
        printf("}\n");
      }
      */
    }
    /*
    int is_terminal = 0;

    size_t terminal_id, non_terminal_id;

    for (size_t i = 0; i < p->terminals_count; ++i) {
      if (strcmp(p->terminals[i]->label, t.lexeme) == 0) {
        is_terminal = 1;
        terminal_id = i;
        break;
      }
    }
    if (is_terminal == 0) {
      for (size_t i = 0; i < p->non_terminals; ++i) {
        if (strcmp(p->non_terminals[i]->label, t.lexeme) == 0) {
          non_terminal_id = i;
        }
      }
    } 
    else { // is terminal = 1
      // Go with the first action even if there's a conflict (TODO: Deal with conflicts)
      if (p->states[current_state]->action_table[terminal_id].actions[0].type == 0) { // Shift
        strcpy(parser_stack[parser_stack_top], p->terminals[terminal_id]->label);
        parser_stack_top++;

        strcpy(parser_stack[parser_stack_top], p->states[current_state]->action_table[terminal_id].actions[0].number);
        parser_stack_top++;
      }
    } */

    step++;
  }
}

void sparser_print_table(sParser *p)
{
  /*
  const char *padding = "                            ";
  printf("[%*.*s%s]", 8, 8, padding, "hell");
  printf("%.*s", 8, "lol");
  printf("%.*s", 8, "cat");
  */
  printf("State\t|Action");
  for (size_t i = 0; i <= p->terminals_count; ++i) printf("\t");
  printf("|Goto");
  for (size_t i = 0; i < p->non_terminals_count; ++i) printf("\t");
  printf("|\n");

  printf("\t|");

  for (size_t i = 0; i < p->terminals_count; ++i) {
    printf("%s\t|", p->terminals[i]->label);
  }
  printf("$\t|");
  for (size_t i = 0; i < p->non_terminals_count; ++i) {
    printf("%s\t|", p->non_terminals[i]->label);
  }

  printf("\n\n");

  for (size_t i = 0; i < p->state_count; ++i) {
    printf("%zu\t|", i);
    for (size_t j = 0; j <= p->terminals_count; ++j) {
      if (p->states[i]->action_table[j].action_count == 0) {
        printf("\t|");
      } else {
        for (size_t k = 0; k < p->states[i]->action_table[j].action_count; ++k) {
          if (p->states[i]->action_table[j].actions[k].type == 0) {
            printf("s");
            printf("%d", p->states[i]->action_table[j].actions[k].number);
          }
          else if (p->states[i]->action_table[j].actions[k].type == 1 &&
                   p->states[i]->action_table[j].actions[k].number == 0) {
                    printf("accept");
                    continue;
                   }
          else if (p->states[i]->action_table[j].actions[k].type == 1) {
            printf("r");
            printf("%d", p->states[i]->action_table[j].actions[k].number);
          }

          if (k < p->states[i]->action_table[j].action_count - 1) printf("/");
        }
        printf("\t|");
      }
    }

    for (size_t j = 0; j < p->non_terminals_count; ++j) {
      if (p->states[i]->goto_table[j] == -1) {
        printf("\t|");
      } else {
        printf("%d\t|", p->states[i]->goto_table[j]);
      }
    }
    printf("\n");
  }
  /*
  printf("%-8s|", "State");
  const char *padding = "                                                                             ";
  int goto_padding_len = p->terminals_count*8 + p->terminals_count - 1 - strlen("Goto");
  int goto_first_padding_len, goto_second_padding_len;
  if (goto_padding_len % 2 == 1) {
    goto_first_padding_len = goto_padding_len/2;
    goto_second_padding_len = goto_padding_len/2 + 1;
  } else {
    goto_first_padding_len = goto_padding_len/2;
    goto_second_padding_len = goto_padding_len/2;
  }
  printf("%*.*s%s%*.*s|", goto_first_padding_len, goto_first_padding_len, padding, "Goto", goto_second_padding_len, goto_second_padding_len, padding);

  int action_padding_len = p->non_terminals_count*8 + p->non_terminals_count - 1 - strlen("Action");
  int action_first_padding_len, action_second_padding_len;
  if (action_padding_len % 2 == 1) {
    action_first_padding_len = action_padding_len/2;
    action_second_padding_len = action_padding_len/2 + 1;
  } else {
    action_first_padding_len = action_padding_len/2;
    action_second_padding_len = action_padding_len/2;
  }
  printf("%*.*s%s%*.*s|\n", action_first_padding_len, action_first_padding_len, padding, "Action", action_second_padding_len, action_second_padding_len, padding);

  printf("%-8s|", "");

  for (size_t i = 0; i < p->terminals_count; ++i) {
    printf("%-8s|", p->terminals[i]->label);
  }
  for (size_t i = 0; i < p->non_terminals_count; ++i) {
    printf("%-8s|", p->non_terminals[i]->label);
  }
  printf("\n");

  for (size_t i = 0; i < p->state_count; ++i) {
    printf("%-8zu|", i);
    for (size_t j = 0; j <= p->terminals_count; ++j) {
      if (p->states[i]->action_table[j].action_count == 0) {
        printf("%-8s|", "");
      } else {
        for (size_t k = 0; k < p->states[i]->action_table[j].action_count; ++k) {
          
        }
      }
    }
  }
  */
}

void sparser_print_ast(AST *ast, size_t level)
{
  if (ast == NULL) return;
  printf("%s\n", ast->text);
  for (size_t i = 0; i < ast->no_of_children; ++i) {
    for (size_t j = 0; j < level; ++j) {
      printf("       ");
    }
    printf("|----- ");
    sparser_print_ast(ast->child[i], level + 1);
  }
}
