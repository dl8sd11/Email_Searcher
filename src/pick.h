#pragma once
#include <math.h>
#include "io.h"
#include <string.h>
#include "helper.h"

void initGroup (SimilarGroup* group, int mid) {
  group->mId = mid;
  group->qSz = 0;
  group->qIds = (int*)malloc(sizeof(int) * 16);
  group->cap = 16;
  group->score = 0;
}

SimilarGroup* pickSimilar (Data *data) {
  int n_mails = data->n_mails, n_queries = data->n_queries;
  SimilarGroup* groups = (SimilarGroup*)malloc(sizeof(SimilarGroup) * n_mails);

  for (int i=0; i<n_mails; i++) {
    initGroup(groups + i, i);
  }

  for (int i=0; i<n_queries; i++) {
    if (data->queries[i].type != find_similar) continue;
    int mid = data->queries[i].data.find_similar_data.mid;
    int qid = data->queries[i].id;

    push_back(&groups[mid].qIds, qid, &groups[mid].cap, &groups[mid].qSz);
    groups[mid].score += data->queries[i].reward;
  }

  qsort(groups, n_mails, sizeof(SimilarGroup), groupComp);
  return groups;
}
