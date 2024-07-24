#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>

#define TOKENS_INIT_CAP 50

typedef enum {
  TOKEN_NUM,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_MULT,
  TOKEN_DIV,
  TOKEN_POW,
  TOKEN_MOD,
  TOKEN_PAREN_OPEN,
  TOKEN_PAREN_CLOSED,
} TokenKind;

typedef struct {
  TokenKind kind;
  double value;
  size_t index;
} Token;

typedef struct {
  Token* ptr;
  size_t len;
  size_t cap;
} Tokens;

void tokenize(const char* arg);
void validate(const char* arg);
double evaluate(const char* arg);

void append_token(Token token);
Token get_token(size_t i);
void print_error(const char* arg, size_t index, const char* msg);
void print_manual(void);

Tokens tokens = {0};

int main(int argc, char** argv) {
  if (argc > 2) {
    fprintf(
      stderr,
      "Expected no argument for the manual or a single argument for a calculation, got %d\n"
      "Tip: Wrap your calculation in double quotes\n",
      argc
    );
    return 1;
  }

  if (argc == 1) {
    print_manual();
    return 0;
  }

  const char* arg = argv[1];
  if (strlen(arg) == 0) {
    fprintf(stderr, "Received an empty argument\n");
    print_manual();
    return 1;
  }

  tokens.ptr = malloc(sizeof (Token) * TOKENS_INIT_CAP);
  if (tokens.ptr == NULL) {
    fprintf(stderr, "Failed to allocate initial memory for tokens\n");
    return 1;
  }
  tokens.cap = TOKENS_INIT_CAP;

  tokenize(arg);

  validate(arg);

  double result = evaluate(arg);

  if (result == 0) {
    printf("0\n");
  } else {
    printf("%.15g\n", result);
  }

  free(tokens.ptr);
  return 0;
}

void tokenize(const char* arg) {
  size_t index = 0;
  while (arg[index] != '\0') {
    if (arg[index] == ' ' || arg[index] == '\t' || arg[index] == '\n') {
      index++;
    } else if (
      (arg[index] == '-' || arg[index] == '+') &&
      (arg[index+1] >= '0' && arg[index+1] <= '9')
    ) {
      char* end;
      double value = strtod(arg + index, &end);
      append_token((Token){TOKEN_NUM, value, index});
      index = end - arg;
    } else if (arg[index] >= '0' && arg[index] <= '9') {
      char* end;
      double value = strtod(arg + index, &end);
      append_token((Token){TOKEN_NUM, value, index});
      index = end - arg;
    } else {
      TokenKind kind;
      switch (arg[index]) {
        case '+': kind = TOKEN_PLUS; break;
        case '-': kind = TOKEN_MINUS; break;
        case '*': kind = TOKEN_MULT; break;
        case '/': kind = TOKEN_DIV; break;
        case '^': kind = TOKEN_POW; break;
        case '%': kind = TOKEN_MOD; break;
        case '(': kind = TOKEN_PAREN_OPEN; break;
        case ')': kind = TOKEN_PAREN_CLOSED; break;
        default:
          print_error(arg, index, "Encountered invalid operator");
          free(tokens.ptr);
          exit(1);
      }
      append_token((Token){kind, 0, index});
      index++;
    }
  }
}

void validate(const char* arg) {
  if (get_token(0).kind == TOKEN_PAREN_CLOSED) {
    print_error(arg, 0, "Encountered invalid parenthesis placement");
    free(tokens.ptr);
    exit(1);
  }
  if (get_token(tokens.len - 1).kind == TOKEN_PAREN_OPEN) {
    print_error(
      arg,
      get_token(tokens.len - 1).index,
      "Encountered invalid parenthesis placement"
    );
    free(tokens.ptr);
    exit(1);
  }

  int paren_count = 0;
  size_t* open_paren_indices = malloc(sizeof (size_t) * tokens.len);
  size_t open_paren_count = 0;
  
  for (size_t i = 0; i < tokens.len; i++) {
    Token cur_token = get_token(i);
    switch (cur_token.kind) {
      case TOKEN_PAREN_OPEN:
        if (i != 0) {
          if (get_token(i - 1).kind == TOKEN_NUM) {
            print_error(
              arg,
              get_token(i - 1).index,
              "Encountered invalid number placement before opening parenthesis"
            );
            free(tokens.ptr);
            free(open_paren_indices);
            exit(1);
          }
        }
        paren_count++;
        open_paren_indices[open_paren_count++] = cur_token.index;
        break;
      case TOKEN_PAREN_CLOSED:
        if (i != tokens.len - 1) {
          if (get_token(i + 1).kind == TOKEN_NUM) {
            print_error(
              arg,
              get_token(i + 1).index,
              "Encountered invalid number placement after closing parenthesis"
            );
            free(tokens.ptr);
            free(open_paren_indices);
            exit(1);
          }
        }
        paren_count--;
        if (paren_count < 0) {
          print_error(
            arg,
            cur_token.index,
            "Encountered unopened closing paranthesis"
          );
          free(tokens.ptr);
          free(open_paren_indices);
          exit(1);
        } else if (open_paren_count > 0) {
          open_paren_count--;
        }
        break;
      case TOKEN_PLUS:
      case TOKEN_MINUS:
      case TOKEN_MULT:
      case TOKEN_DIV:
      case TOKEN_POW:
      case TOKEN_MOD: {
        if (i == 0) {
          print_error(
            arg,
            get_token(i).index,
            "Encountered invalid operator placement"
          );
          free(tokens.ptr);
          free(open_paren_indices);
          exit(1);
        } else {
          switch (get_token(i - 1).kind) {
            case TOKEN_PLUS:
            case TOKEN_MINUS:
            case TOKEN_MULT:
            case TOKEN_DIV:
            case TOKEN_POW:
              print_error(
                arg,
                get_token(i - 1).index,
                "Encountered invalid operator placement"
              );
              free(tokens.ptr);
              free(open_paren_indices);
              exit(1);
            default: break;
          }
        }
        if (i == tokens.len - 1) {
          print_error(
            arg,
            get_token(i).index,
            "Encountered invalid operator placement"
          );
          free(tokens.ptr);
          free(open_paren_indices);
          exit(1);
        } else {
          TokenKind next_token = get_token(i + 1).kind;
          if (!(next_token == TOKEN_NUM || next_token == TOKEN_PAREN_OPEN)) {
            print_error(
              arg,
              get_token(i + 1).index,
              "Encountered invalid operator placement"
            );
            free(tokens.ptr);
            free(open_paren_indices);
            exit(1);
          }
        }
      } break;
      case TOKEN_NUM:
        if (
          (i > 0 && get_token(i - 1).kind == TOKEN_NUM) ||
          (i < tokens.len - 1 && get_token(i + 1).kind == TOKEN_NUM)
        ) {
            print_error(
              arg,
              get_token(i).index,
              "Encountered invalid number placement\nNOTE: Remember to seperate numbers and operators with whitespace"
            );
            free(tokens.ptr);
            free(open_paren_indices);
            exit(1);
        }
        break;
    }
  }

  if (open_paren_count > 0) {
    print_error(
      arg,
      open_paren_indices[open_paren_count - 1],
      "Encountered unclosed opening parenthesis"
    );
    free(tokens.ptr);
    free(open_paren_indices);
    exit(1);
  }

  free(open_paren_indices);
}

double evaluate(const char* arg) {
  static size_t index = 0;
  double left = 0;
  Token cur_token = get_token(index);

  if (cur_token.kind == TOKEN_PAREN_OPEN) {
    index++;
    left = evaluate(arg);
    index++;
    cur_token = get_token(index);
  } else if (cur_token.kind == TOKEN_NUM) {
    left = cur_token.value;
    index++;
    cur_token = get_token(index);
  }

  while (index < tokens.len) {
    if (cur_token.kind == TOKEN_PAREN_CLOSED) return left;

    TokenKind cur_op = cur_token.kind;
    size_t cur_op_arg_index = get_token(index).index;
    index++;

    cur_token = get_token(index);
    double right = 0;
    
    if (cur_token.kind == TOKEN_PAREN_OPEN) {
      index++;
      right = evaluate(arg);
      index++;
      cur_token = get_token(index);
    } else if (cur_token.kind == TOKEN_NUM) {
      right = cur_token.value;
      index++;
      cur_token = get_token(index);
    }

    switch (cur_op) {
      case TOKEN_PLUS:
        left += right;
        break;
      case TOKEN_MINUS:
        left -= right;
        break;
      case TOKEN_MULT:
        left *= right;
        break;
      case TOKEN_DIV:
        if (right == 0) {
          print_error(arg, cur_op_arg_index, "Division by zero error");
          free(tokens.ptr);
          exit(1);
        }
        left /= right;
        break;
      case TOKEN_POW:
        left = pow(left, right);
        break;
      case TOKEN_MOD:
        if (right == 0) {
          print_error(arg, cur_op_arg_index, "Modulo by zero error");
          free(tokens.ptr);
          exit(1);
        }
        left = fmod(left, right);
        break;
      default: break;
    }
  }

  return left;
}

void append_token(Token token) {
  if (tokens.len + 1 > tokens.cap) {
    size_t new_cap = tokens.cap * 2;
    Token* new_ptr = realloc(tokens.ptr, new_cap);
    if (new_ptr == NULL) {
      fprintf(stderr, "Failed to reallocate memory for tokens\n");
      free(tokens.ptr);
      exit(1);
    }
    tokens.ptr = new_ptr;
    tokens.cap = new_cap;
  }

  tokens.ptr[tokens.len++] = token;
}

Token get_token(size_t i) {
  return tokens.ptr[i];
}

void print_error(const char* arg, size_t index, const char* msg) {
  fprintf(stderr, "%s\n%s\n", msg, arg);
  for (size_t i = 0; i < index; i++) {
    fprintf(stderr, " ");
  }
  fprintf(stderr, "^\n");
}

void print_manual(void) {
  fprintf(stderr, "Usage:\n\tcalc \"[expression]\"\n");
  fprintf(stderr, "Example:\n\tcalc \"(5 + -4.25) / (-2 - ((3.4 ^ 3) * 0.1)) %% 2\"\n");
  fprintf(stderr, "Operators:\n");
  fprintf(stderr, "\t- Addition:       +\n");
  fprintf(stderr, "\t- Subtraction:    -\n");
  fprintf(stderr, "\t- Multiplication: *\n");
  fprintf(stderr, "\t- Division:       /\n");
  fprintf(stderr, "\t- Exponentiation: ^\n");
  fprintf(stderr, "\t- Modulus:        %%\n");
}
