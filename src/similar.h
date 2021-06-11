#pragma once
#include "io.h"
#include "../api.h"
#include "group.h"
#ifndef DBG
#define fprintf(...)
#endif

int hashInSortedArray (int hash, int* array, int sz) {
  int L = -1, R = sz;
  while (L < R - 1) {
    int M = (L + R) >> 1;
    if (array[M] < hash) L = M;
    else R = M;
  }
  return R < sz && array[R] == hash;
}

int calcIntersection (int x, int y, TokenHash* mail_hash) {
  int res = 0;
  if (mail_hash->len[x] > mail_hash->len[y]) {
    int tmp = x;
    x = y;
    y = tmp;
  }

  for (int i=0; i<mail_hash->len[x]; i++) {
    res += hashInSortedArray(mail_hash->hash[x][i], mail_hash->hash[y], mail_hash->len[y]);
  }

  return res;
}

typedef struct similar_data {
  int id;
  double jaccard;
} SimilarData;



int jacComp (const void *a, const void *b) {
  return ((SimilarData*)b)->jaccard - ((SimilarData*)a)->jaccard > 0 ? 1 : -1;
}

void querySimilar (Data *data, SimilarGroup* group, TokenHash* mail_hash) {
  fprintf(stderr, "mail %d, %d queries\n", group->mId, group->qSz);
  int target = group->mId;
  
  static int intersection[10004];
  static SimilarData res[10004];

  for (int i=0; i<data->n_mails; i++) {
    if (i != target) intersection[i] = calcIntersection(i, target, mail_hash);
    else intersection[i] = -1;

    res[i].id = i;
    res[i].jaccard = intersection[i] / (double)(mail_hash->len[i] + mail_hash->len[target] - intersection[i]);
  }




  fprintf(stderr, "%d %lf\n", data->n_mails, res[869].jaccard);
  for (int i=0; i<100; i++) {
    fprintf(stderr, "%lf ", res[i].jaccard);
  }
  fprintf(stderr, "\n");
  qsort(res, data->n_mails, sizeof(SimilarData), jacComp);


  static int ans[10004];
  for (int i=0; i<group->qSz; i++) {
    int qid = group->qIds[i];
    double threshold = data->queries[qid].data.find_similar_data.threshold;
    fprintf(stderr, "qid: %d %lf\n", qid, threshold);
    int asz = 0;

    for (int j=0; res[j].jaccard > threshold; j++) {
      ans[asz++] = res[j].id;
    }
    qsort(ans, asz, sizeof(int), comp);

    for (int j=0; j<asz; j++) {
      fprintf(stderr, "%d ", ans[j]);
    }
    fprintf(stderr, "\n");
    api.answer(qid, ans, asz);
  }
}
