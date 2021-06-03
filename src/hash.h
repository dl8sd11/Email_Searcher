#pragma once
unsigned int hash1(char *str);
unsigned int hash2(char *str);
#include <stdio.h>
#include <string.h>
unsigned int hash1(char *str){// bkdr
    int n = strlen(str);
    unsigned int seed = 131; 
    unsigned int hash = 0;
    while (n--){
        hash = hash * seed + (*str++);
    }
    return hash;
}
unsigned int hash2(char *str){// djb2
    int n = strlen(str);
    unsigned int hash = 5381;
    while (n--){   
        hash = ((hash << 5) + hash) + (*str++);
    }
    return hash;
}
