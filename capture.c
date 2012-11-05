#include "regex.h"

Capture *free_capture;

Capture *newCapture(size_t size)
{
  Capture *c;
  if (free_capture) {
    c = free_capture;
    free_capture = (Capture*)free_capture->captures[0];
  } else
    c = malloc(sizeof(Capture));
  c->size = size;
  c->ref = 1;
  return c;
}

Capture *incRef(Capture *c)
{
  c->ref++;
  return c;
}

Capture *decRef(Capture *c)
{
  if (! --c->ref) {
    c->captures[0] = (char*)free_capture;
    free_capture = c;
  }
  return c;
}

Capture *updateCapture(Capture *c, int i, const char *pos)
{
  if (c->ref > 1) {
    Capture *cc = newCapture(c->size);
    memcpy(cc->captures, c->captures, sizeof(*c->captures) * c->size);
    c->ref--;
    c = cc;
  }
  c->captures[i] = pos;
  return c;
}
