#include <assert.h>
#include "regex.h"

static Instruction *pc;

static size_t count(Regex *r)
{
  switch (r->type) {
    case Lit:
    case Dot:
      return 1;
    case Paren:
    case Star: case NGStar:
      return 2 + count(r->left);
    case Optional: case NGOptional:
    case Plus: case NGPlus:
      return 1 + count(r->left);
    case Concat:
      return count(r->left) + count(r->right);
    case Alternative:
      return 2 + count(r->left) + count(r->right);
    default:
      assert(0);
  }
}

static void build(Regex *r)
{
  switch (r->type) {
    case Lit:
      pc->op = Char;
      pc++->ch = r->ch;
      break;
    case Dot:
      pc++->op = Any;
      break;
    case Concat:
      build(r->left);
      build(r->right);
      break;
    case Alternative:
      {
        Instruction *x = pc++;
        x->op = Split;
        x->left = pc;
        build(r->left);
        x->right = pc + 1;
        Instruction *y = pc++;
        y->op = Jmp;
        build(r->right);
        y->left = pc;
        break;
      }
    case Optional:
    case NGOptional:
      {
        Instruction *x = pc++;
        x->op = Split;
        x->left = pc;
        build(r->left);
        x->right = pc;
        if (r->type == NGOptional)
          swap(x->left, x->right);
        break;
      }
    case Star:
    case NGStar:
      {
        Instruction *x = pc++;
        x->op = Split;
        x->left = pc;
        build(r->left);
        pc->op = Jmp;
        pc++->left = x;
        x->right = pc;
        if (r->type == NGStar)
          swap(x->left, x->right);
        break;
      }
    case Plus:
    case NGPlus:
      {
        Instruction *x = pc;
        build(r->left);
        pc->op = Split;
        pc->left = x;
        pc->right = pc + 1;
        if (r->type == NGPlus)
          swap(pc->left, pc->right);
        pc++;
        break;
      }
    case Paren:
      pc->op = Save;
      pc++->save_id = 2 * r->capture_id;
      build(r->left);
      pc->op = Save;
      pc++->save_id = 2 * r->capture_id + 1;
      break;
  }
}

Program *compile(Regex *r)
{
  size_t size = count(r) + 1;
  Program *p = malloc(sizeof(Program) + size * sizeof(Instruction));
  pc = p->start = (Instruction*)(p+1);
  build(r);
  (pc++)->op = Match;
  p->size = pc - p->start;
  return p;
}

void printProgram(Program *p)
{
  Instruction *s = p->start;
  for (size_t i = 0; i < p->size; i++) {
    Instruction *pc = s + i;
    printf("%2zd ", i);
    switch (pc->op) {
      case Char:
        printf("char %c\n", pc->ch);
        break;
      case Any:
        puts("any");
        break;
      case Split:
        printf("split %td, %td\n", pc->left - s, pc->right - s);
        break;
      case Save:
        printf("save %d\n", pc->save_id);
        break;
      case Jmp:
        printf("jmp %td\n", pc->left - s);
        break;
      case Match:
        puts("match");
        break;
      default:
        abort();
        break;
    }
  }
}
