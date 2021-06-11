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

long long hash3 (char *str) {
  unsigned int h1 = hash1(str);
  unsigned int h2 = hash2(str);
  return ((unsigned long long)(h1) << h1) | h2;
}
