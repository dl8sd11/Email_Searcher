#pragma once
int hash1(char *str);
int hash2(char *str);
#include <stdio.h>
#include <string.h>
int hash1(char *str){// bkdr
  unsigned int seed = 131; 
  unsigned int hash = 0;
  while (*str){
    hash = hash * seed + (*str++);
  }
  return (int)hash;
}
int hash2(char *str){// djb2
  unsigned int hash = 5381;
  while (*str){   
    hash = ((hash << 5) + hash) + (*str++);
  }
  return (int)hash;
}

#define HASH_TYPE unsigned long long
const HASH_TYPE mod = (1ull<<61) - 1;
HASH_TYPE modmul(HASH_TYPE a, HASH_TYPE b){
	HASH_TYPE l1 = (int)a, h1 = a>>32, l2 = (int)b, h2 = b>>32;
	HASH_TYPE l = l1*l2, m = l1*h2 + l2*h1, h = h1*h2;
	HASH_TYPE ret = (l&mod) + (l>>61) + (h << 3) + (m >> 29) + (m << 35 >> 3) + 1;
	ret = (ret & mod) + (ret>>61);
	ret = (ret & mod) + (ret>>61);
	return ret-1;
}

HASH_TYPE hash3 (char *str) {
  HASH_TYPE base = 1;
  const HASH_TYPE c = 880301;
  HASH_TYPE sum = 0;
  while (*str) {
    base = modmul(base, c);
    HASH_TYPE cur = modmul(base, *str);
    sum = sum + cur >= mod ? sum + cur - mod : sum + cur;
    str++;
  }
  return sum;
}
