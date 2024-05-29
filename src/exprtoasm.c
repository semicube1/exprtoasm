#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../include/slex.h"
#include "../include/sparser.h"

#define MAX_SOURCE_STR_LEN 1024

bool verbose_flag = false;

void generate_expr_code(FILE *stream, const AST *ast)
{
  if (ast->no_of_children == 0) {
    fprintf(stream, "\tmovq $%s, %%rax\n"
                    "\tpushq %%rax\n\n", ast->text);
  } 
  else {
    generate_expr_code(stream, ast->child[1]);
    generate_expr_code(stream, ast->child[0]);

    fprintf(stream, "\tpopq %%rbx\n" 
                    "\tpopq %%rax\n\n");

    if (strcmp(ast->text, "+") == 0) fprintf(stream, "\taddq %%rbx, %%rax\n");
    else if (strcmp(ast->text, "-") == 0) fprintf(stream, "\tsubq %%rbx, %%rax\n");
    else if (strcmp(ast->text, "*") == 0) fprintf(stream, "\tmul %%rbx\n");
    else if (strcmp(ast->text, "/") == 0) fprintf(stream, "\tdiv %%rbx\n");

    fprintf(stream, "\tpushq %%rax\n\n");
  }
}

void generate_target_code(FILE *stream, const AST *ast)
{
  fprintf(stream, "format_str:\n"
                  "\t.string \"%%ld\\n\"\n\n"
                  ".text\n"
                  "\t.globl main\n"
                  "\t.type main, @function\n\n"
                  "main:\n"
                  "\tpushq %%rbp\n"
                  "\tmovq %%rsp, %%rbp\n\n");

  if (ast != NULL) {
    generate_expr_code(stream, ast);
  }

  fprintf(stream, "\tpopq %%rbx\n"
                  "\tmovq %%rbx, %%rsi\n"
                  "\tleaq format_str(%%rip), %%rdi\n"
                  "\tmovq $0, %%rax\n"
                  "\tcall printf\n\n"
                  "\tmovl $0, %%eax\n"
                  "\tpopq %%rbp\n"
                  "\tret\n\n"
                  ".section .note.GNU-stack,\"\",@progbits\n");
}

void print_usage(FILE *stream, const char *program)
{
  fprintf(stream, "Usage: %s [-v | -h] <expression>. Output will be saved to `output.s`\n", program);
}

int main(int argc, char *argv[])
{
  //char input_path[256] = {0};
  static char source_str[MAX_SOURCE_STR_LEN + 1];

  if (argc > 1) {
    if (strcmp(argv[1], "-v") == 0) {
      verbose_flag = true; 
      if (argc == 2) {
        print_usage(stdout, argv[0]);
        fprintf(stderr, "Error: No Expression provided\n");
        exit(1);
      }
      else {
        strcpy(source_str, argv[2]);
        printf("Got expression: %s\n", source_str);
      }
    }
    else if (strcmp(argv[1], "-h") == 0) {
      print_usage(stdout, argv[0]);
      exit(0);
    }
    else {
      strcpy(source_str, argv[1]);
      printf("Got expression: %s\n", source_str);
    }
  } else {
      print_usage(stdout, argv[0]);
      fprintf(stderr, "Error: No Expression provided\n");
      exit(1);
  }

  sLex *l = slex_new();

  //fgets(source_str, sizeof(source_str), stdin);

  slex_add_token_kind(l, "\\+", "ADD");
  slex_add_token_kind(l, "\\-", "SUB");
  slex_add_token_kind(l, "\\*", "MUL");
  slex_add_token_kind(l, "\\/", "DIV");
  slex_add_token_kind(l, "\\(", "L_PAR");
  slex_add_token_kind(l, "\\)", "R_PAR");
  slex_add_token_kind(l, "[0-9]+", "NUM");

  slex_add_source_str(l, source_str);

  sParser *p = sparser_new();

  sparser_add_terminal(p, "ADD");
  sparser_add_terminal(p, "SUB");
  sparser_add_terminal(p, "MUL");
  sparser_add_terminal(p, "DIV");
  sparser_add_terminal(p, "L_PAR");
  sparser_add_terminal(p, "R_PAR");
  sparser_add_terminal(p, "NUM");

  sparser_add_bnf_rule(p, "E -> E ADD T", "node 2 1 2");
  sparser_add_bnf_rule(p, "E -> E SUB T", "node 2 1 2");
  sparser_add_bnf_rule(p, "E -> T", "");
  sparser_add_bnf_rule(p, "T -> T MUL F", "node 2 1 2");
  sparser_add_bnf_rule(p, "T -> T DIV F", "node 2 1 2");
  sparser_add_bnf_rule(p, "T -> F", "");
  sparser_add_bnf_rule(p, "F -> L_PAR E R_PAR", "");
  sparser_add_bnf_rule(p, "F -> NUM", "leaf 1");

  sparser_find_first(p);
  sparser_find_states(p);


  if (verbose_flag == true) {
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
  }

  if (verbose_flag == true) printf("\n[STEP-BY-STEP PARSING]\n\n");

  AST *ast = sparser_parse_string(p, l, verbose_flag);

  if (ast == NULL) {
    printf("[INFO] Invalid Input\n");
    exit(1);
  }

  FILE *output = fopen("./output.s", "w");
  generate_target_code(output, ast);
  fclose(output);

  if (verbose_flag == true) {
    printf("\n\n[AST]\n\n");
    sparser_print_ast(ast, 0);
  }

  printf("\n[INFO] Successfully generated target code in `./output.s`\n");

  slex_free(l);
  // sparser_free(p);

  return 0;
}
