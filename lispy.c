/* lispy.c 
 * I put this piece of program here for it is the start to building a programming
 * language, which is fun
 */

#include "mpc.h"

#ifdef _WIN32

static char buffer[2048];

/* Fake readline function */
char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

/* Otherwise include the editline headers */
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

/* Check which operator is used */
long eval_op(char* op, long x, long y) {
  if (strcmp(op, "+") == 0) { return x + y;}
  if (strcmp(op, "-") == 0) { return x - y;}
  if (strcmp(op, "*") == 0) { return x * y;}
  if (strcmp(op, "/") == 0) { return x / y;}
  if (strcmp(op, "%") == 0) { return x % y;}
  return 0;
}

long eval(mpc_ast_t* t) {
  /* Return a number directly */
  if (strstr(t->tag, "number")) {
    return atoi(t->contents);
  }

  char* op = t->children[1]->contents;

  long x = eval(t->children[2]);

  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(op, x, eval(t->children[i]));
    i++;
  }

  return x;
}

int main(int argc, char** argv) {

  /* Create some parsers */
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");

  /* Define the grammar */
  mpca_lang(
    MPCA_LANG_DEFAULT,
    " \
      number : /-?[0-9]+/ ; \
      operator : '+' | '-' | '*' | '/' | '%' ; \
      expr: <number> | '(' <operator> <expr>+ ')' ; \
      lispy : /^/ <operator> <expr>+ /$/ ; \
    ",
    Number, Operator, Expr, Lispy);

  /* Print version and some help */
  puts("Lispy Version 0.0.0.0.3");
  puts("Press Ctrl-C to Exit\n");

  while (1) {
    /* Output the prompt and get input */
    char* input = readline("lispy>> ");

    if (!input) {
      printf("EOF\n");
      return 0;
    }

    /* Add input to history */
    add_history(input);

    /* Attempt to parse the user input */
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {

      long result = eval(r.output);
      printf("%li\n", result);
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  /* Undefine and delete the parsers */
  mpc_cleanup(4, Number, Operator, Expr, Lispy);

  return 0;
}
