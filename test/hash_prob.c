#include "../src/hash.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void gen (char *s) {
  for (int i=0; i<6; i++) {
    s[i] = rand() % 26 + 'a';
  }
  s[6] = 0;
}

int main () {
  char s1[10], s2[10];

  int colli, cnt = 0;
  while (1) {
    gen(s1);
    gen(s2);

    cnt++;
    if (strcmp(s1, s2)) {
      colli += hash2(s1) == hash2(s2);
    }

    if (cnt % 100000 == 9999) {
      printf("Prob: %d / %d\n", colli, cnt);
    }
  }
}
