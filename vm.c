#include <assert.h>
#include <stdlib.h>
#include "regex.h"

typedef struct {
  Instruction *pc;
  Capture *capture;
} State;

typedef struct {
  size_t size;
  State states[0];
} StateList;

int tick;

static StateList* newStateList(size_t size)
{
  StateList *sl = malloc(sizeof(StateList) + size * sizeof(State));
  sl->size = 0;
  return sl;
}

static State state(Instruction *pc, Capture *capture)
{
  State s = {pc, capture};
  return s;
}

static void addState(StateList *sl, State s, const char *pos)
{
  if (s.pc->timestamp == tick) {
    decRef(s.capture);
    return;
  }

  s.pc->timestamp = tick;
  switch (s.pc->op) {
    case Jmp:
      addState(sl, state(s.pc->left, s.capture), pos);
      break;
    case Split:
      addState(sl, state(s.pc->left, incRef(s.capture)), pos);
      addState(sl, state(s.pc->right, s.capture), pos);
      break;
    case Save:
      addState(sl, state(s.pc + 1, updateCapture(s.capture, s.pc->save_id, pos)), pos);
      break;
    default:
      sl->states[sl->size++] = s;
      break;
  }
}

bool run(Program *p, const char *input, const char **captures, size_t ncaptures)
{
  Capture *capture = newCapture(ncaptures), *match = NULL;
  memset(capture->captures, 0, sizeof(*capture->captures) * ncaptures);

  StateList *cur = newStateList(p->size),
            *suc = newStateList(p->size);
  for (size_t i = 0; i < p->size; i++)
    p->start[i].timestamp = 0;
  tick = 1;
  addState(cur, state(p->start, capture), input);

  for (; ; input++) {
    tick++;
    for (size_t i = 0; i < cur->size; i++) {
      Instruction *pc = cur->states[i].pc;
      Capture *capture = cur->states[i].capture;
      switch (pc->op) {
        case Char:
          if (pc->ch == *input)
            addState(suc, state(pc + 1, capture), input + 1);
          else
            decRef(capture);
          break;
        case Any:
          if (*input)
            addState(suc, state(pc + 1, capture), input + 1);
          else
            decRef(capture);
          break;
        case Match:
          if (match) decRef(match);
          match = capture;
          while (++i < cur->size)
            decRef(cur->states[i].capture);
          goto Break;
      }
    }
Break:
    {
      StateList *t = cur;
      cur = suc;
      suc = t;
      suc->size = 0;
      if (! *input)
        break;
    }
  }

  free(cur);
  free(suc);

  if (match) {
    for (size_t i = 0; i < ncaptures; i++)
      captures[i] = match->captures[i];
    decRef(match);
    return true;
  }
  return false;
}
