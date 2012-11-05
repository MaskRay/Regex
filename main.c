#include "regex.h"

int main(int argc, char *argv[])
{
  char s[MAXN];
  if (argc < 2)
    return 1;
  Regex *r = newRegex(argv[1]);
  printRegex(r, 0);

  Program *p = compile(r);
  printProgram(p);
  puts("");
  puts("+++");
  puts("");

  const char *captures[MAXCAP];
  for (int i = 2; i < argc; i++)
    if (! run(p, argv[i], captures, MAXCAP))
      puts("unmatch");
    else {
      puts("match");
      for (size_t i = 0; i < MAXN; i++) {
        if (! captures[i]) break;
        printf("%zd: %td\n", i, captures[i] - argv[2]);
      }
    }

  free(p);
  destroyRegex(r);
  return 0;
}
