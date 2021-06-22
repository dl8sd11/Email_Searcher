#pragma once
#include <math.h>
#include "io.h"
#include "group.h"
#include <string.h>
#include "helper.h"

void compute_prefix(int n_mails, int* mail_token, int* sorted, long long int* log_prefix, long long int* prefix){
  for (int i = 0; i < n_mails; i++)
    sorted[i] = mail_token[i];
  qsort(sorted, n_mails, sizeof(int), comp);

  log_prefix[0] = (int)log(mail_token[0]);
  prefix[0] = mail_token[0];
  for (int i = 1; i < n_mails; i++){
    log_prefix[i] = log_prefix[i-1] + (int)log(mail_token[i]);
    prefix[i] = prefix[i-1] + mail_token[i];
  }
}
void pickProblem(PickOrder *pick_order, TokenHash* mail_hash, Data* data){
  int mailN = data->n_mails, queryN = data->n_queries;
  for (int i = 0; i < queryN; i++)
    pick_order[i].id = i;
  long long int prefix[mailN];
  long long int log_prefix[mailN];
  int sorted[mailN];
  compute_prefix(mailN, mail_hash->len, sorted, log_prefix, prefix);
  for (int i = 0; i < queryN; i++){
    if (data->queries[i].type == expression_match){
      // (token of expression) * (log(token1)+log(token2)...)
      int expr_token = strlen((data->queries[i]).data.expression_match_data.expression) / 3;// bongi's estimate
      pick_order[i].time = expr_token * log_prefix[mailN-1] / (data->queries[i]).reward;
    }   
    if (data->queries[i].type == find_similar){
      //min(tokenA, tokeni) * log(max(tokenA, tokeni))
      // = (token1+...+tokenA-1) * log(tokenA) + tokenA * (log(tokenA+1)+...))
      int l = -1, r = mailN;
      int tokenI = (data->queries[i]).data.find_similar_data.mid;
      int token = mail_hash->len[tokenI];
      while(l < r-1){
        int m = (l+r) / 2;
        if (token <= sorted[m]) r = m;
        else l = m;
      }
      int sortedI = r, timecomp;
      if (sortedI == 0)
        timecomp = token*(log_prefix[mailN-1]-log_prefix[sortedI]);
      else
        timecomp = prefix[sortedI-1]*log(token) + token*(log_prefix[mailN-1]-log_prefix[sortedI]);
      pick_order[i].time = timecomp / (data->queries[i]).reward;
    }
    else{
      // len * log(len)
      int len = (data->queries[i]).data.group_analyse_data.len;
      pick_order[i].time = len*log(len) / (data->queries[i]).reward;
    }
  }
  qsort(pick_order, queryN, sizeof(PickOrder), pickComp);
  // time short -> time long
 
}

void pickOnly(PickOrder *order, Data *data, int tp) {
  int queryN = data->n_queries;
  int* res = (int*)malloc(queryN * sizeof(int));
  int resN = 0;
  for (int i=0; i<queryN; i++) {
    if (data->queries[i].type == tp) res[resN++] = i;
  }
  for (int i=0; i<queryN; i++) {
    if (data->queries[i].type != tp) res[resN++] = i;
  }
  for (int i=0; i<queryN; i++) order[i].id = res[i];
  free(res);
}

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
