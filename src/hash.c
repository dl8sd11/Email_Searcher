#include <stdio.h>
#include <string.h>
#include "hash.h"
unsigned int hash1(char *str, int n){// bkdr
    unsigned int seed = 131; 
    unsigned int hash = 0;
    while (n--){
        hash = hash * seed + (*str++);
    }
    return hash;
}
unsigned int hash2(char *str, int n){// djb2
    unsigned int hash = 5381;
    while (n--){   
        hash = ((hash << 5) + hash) + (*str++);
    }
    return hash;
}
