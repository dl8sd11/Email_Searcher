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
    int** hash; // hash[i][j] repr jth token of ith mail
    int* len;   // len[i] repr number of token of ith mail
} TokenHash;
typedef struct pick_order{
    int id;// ith query
    int time; //time per reward
}PickOrder;

typedef struct similar_group {
  int mId, qSz;
  double score;
  int *qIds, cap;
} SimilarGroup;
