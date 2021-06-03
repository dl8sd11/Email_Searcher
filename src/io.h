#pragma once
#include "../api.h"
typedef struct data {
    int n_mails, n_queries;
    mail *mails;
    query *queries;
} Data;

typedef struct ans {
    int* array;
    int len;
} Ans;

typedef struct token_hash {
    int** hash;
    int* len;
} TokenHash;
