#ifndef REGEX_H
#define REGEX_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXN 92
#define MAXCAP 20
#define swap(x,y) do \
   { unsigned char swap_temp[sizeof(x) == sizeof(y) ? (signed)sizeof(x) : -1]; \
     memcpy(swap_temp,&y,sizeof(x)); \
     memcpy(&y,&x,       sizeof(x)); \
     memcpy(&x,swap_temp,sizeof(x)); \
   } while(0)

enum Token {
  Lit = 256,
  Dot,

  Sentinel,
  Paren,
  RParen,

  Optional,
  Star,
  Plus,
  NGOptional,
  NGStar,
  NGPlus,

  Concat,
  Alternative,
};

enum Op {
  Char,
  Any,
  Save,
  Match,
  Split,
  Jmp,
};

enum TokenType {
  NonOp,
  Surround,
  Unary,
  Binary,
};

static const enum TokenType token_type[Alternative + 1] = {
  [Char] = NonOp,
  [Dot] = NonOp,

  [Optional] = Unary,
  [Star] = Unary,
  [Plus] = Unary,
  [NGOptional] = Unary,
  [NGStar] = Unary,
  [NGPlus] = Unary,

  [Concat] = Binary,
  [Alternative] = Binary,

  [Sentinel] = Surround,
  [Paren] = Surround,
};

typedef struct Regex Regex;
struct Regex
{
  enum Token type;
  union {
    char ch;
    int capture_id;
  };
  Regex *left, *right;
};

typedef struct Instruction Instruction;
struct Instruction
{
  enum Op op;
  union {
    char ch;
    int save_id;
  };
  int timestamp;
  Instruction *left, *right;
};

typedef struct Program Program;
struct Program
{
  Instruction *start;
  size_t size;
};

typedef struct Capture Capture;
struct Capture
{
  size_t ref, size;
  const char *captures[MAXCAP];
};

Regex *newRegex(const char*);
void printRegex(Regex *, int);
void destroyRegex(Regex *r);

Program *compile(Regex *);
void printProgram(Program *);

Capture *newCapture(size_t);
Capture *incRef(Capture *);
Capture *decRef(Capture *);
Capture *updateCapture(Capture *, int, const char *);

bool run(Program *, const char *, const char **, size_t);

#endif /* end of include guard: REGEX_H */
