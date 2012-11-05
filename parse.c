#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "regex.h"

static const int isp[Alternative + 1] = {
  [Sentinel] = 1,
  [Paren] = 2,
  [Alternative] = 4,
  [Concat] = 6,
  [Optional] = 8,
  [Star] = 8,
  [Plus] = 8,
  [NGOptional] = 8,
  [NGStar] = 8,
  [NGPlus] = 8,
};
static const int icp[Alternative + 1] = {
  [Sentinel] = 1,
  [Paren] = 9,
  [Alternative] = 3,
  [Concat] = 5,
  [Optional] = 7,
  [Star] = 7,
  [Plus] = 7,
  [NGOptional] = 7,
  [NGStar] = 7,
  [NGPlus] = 7,
  [RParen] = 2,
};
static const enum Token c2t[256] = {
  ['\0'] = Sentinel,
  ['('] = Paren,
  [')'] = RParen,
  ['?'] = Optional,
  ['+'] = Plus,
  ['*'] = Star,
  ['|'] = Alternative,
};

static Regex *newObj(int type, Regex *left, Regex *right)
{
  Regex *r = malloc(sizeof(Regex));
  r->type = type;
  r->left = left;
  r->right = right;
  r->capture_id = 0; // no use
  return r;
}

void destroyRegex(Regex *r)
{
  switch (token_type[r->type]) {
    case Surround:
    case Unary:
      destroyRegex(r->left);
      break;
    case Binary:
      destroyRegex(r->left);
      destroyRegex(r->right);
      break;
  }
  free(r);
}

Regex *newRegex(const char *s)
{
  static Regex *obj[MAXN];
  static enum Token op[MAXN];
  enum Token tok;
  bool flag = false, quit = false;
  int ic, nobj = 0, nop = 0, nparen = 0;
  op[nop++] = Sentinel;
  while (! quit) {
    // implicit concat
    tok = *s;
    if (c2t[tok]) tok = c2t[tok];
    if (flag && (tok < 256 || tok == Paren)) {
      flag = false;
      tok = Concat;
      ic = icp[Concat];
    } else {
      s++;
      ic = icp[tok];
      flag = tok != Paren && tok != Alternative;
      if (tok == Sentinel)
        quit = true;
    }

    if (! ic) {
      if (tok == '.')
        obj[nobj++] = newObj(Dot, NULL, NULL);
      else {
        obj[nobj] = newObj(Lit, NULL, NULL);
        obj[nobj++]->ch = tok;
      }
      continue;
    }

    if (! quit && *s == '?') {
      if (tok == Optional)
        tok = NGOptional, s++;
      else if (tok == Star)
        tok = NGStar, s++;
      else if (tok == Plus)
        tok = NGPlus, s++;
    }

    bool new_obj = true;
    for (; nop > 0 && isp[op[nop-1]] >= ic; nop--) {
      switch (token_type[op[nop-1]]) {
        case Surround:
          if (op[nop-1] == Paren) {
            obj[nobj-2]->left = obj[nobj-1];
            nobj--;
          }
          break;
        case Unary:
          break;
        case Binary:
          obj[nobj-2]->right = obj[nobj-1];
          nobj--;
          break;
      }
      if (isp[op[nop-1]] == ic) {
        new_obj = false;
        nop--;
        break;
      }
    }
    if (new_obj) {
      op[nop++] = tok;
      if (tok == Paren) {
        obj[nobj] = newObj(tok, NULL, NULL);
        obj[nobj++]->capture_id = nparen++;
      } else // unary or binary
        obj[nobj-1] = newObj(tok, obj[nobj-1], NULL);
    }
  }
  assert(nop == 0 && nobj == 1);
  //return obj[0];
  // implicit .*?
  return newObj(Concat, newObj(NGStar, newObj(Dot, NULL, NULL), NULL), newObj(Paren, obj[0], NULL));
}

void printRegex(Regex *r, int d)
{
  switch (r->type) {
    case Alternative:
      printf("%*sAlternative\n", 2*d, "");
      printRegex(r->left, d + 1);
      printRegex(r->right, d + 1);
      break;
    case Concat:
      printf("%*sConcat\n", 2*d, "");
      printRegex(r->left, d + 1);
      printRegex(r->right, d + 1);
      break;
    case Lit:
      printf("%*sLit %c\n", 2*d, "", r->ch);
      break;
    case Dot:
      printf("%*sDot\n", 2*d, "");
      break;
    case Paren:
      printf("%*sParen\n", 2*d, "");
      printRegex(r->left, d + 1);
      break;
    case Optional:
      printf("%*sOptional\n", 2*d, "");
      printRegex(r->left, d + 1);
      break;
    case NGOptional:
      printf("%*sNGOptional\n", 2*d, "");
      printRegex(r->left, d + 1);
      break;
    case Star:
      printf("%*sStar\n", 2*d, "");
      printRegex(r->left, d + 1);
      break;
    case NGStar:
      printf("%*sNGStar\n", 2*d, "");
      printRegex(r->left, d + 1);
      break;
    case Plus:
      printf("%*sPlus\n", 2*d, "");
      printRegex(r->left, d + 1);
      break;
    case NGPlus:
      printf("%*sNGPlus\n", 2*d, "");
      printRegex(r->left, d + 1);
      break;
    default:
      break;
  }
}
