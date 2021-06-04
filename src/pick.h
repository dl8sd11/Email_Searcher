#pragma once
#include <math.h>
#include "io.h"
#include "group.h"

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
int pickComp(const void *a, const void *b){
    PickOrder A = *(PickOrder *)a, B = *(PickOrder *)b;
    if (A.time > B.time) return 1;
    if (A.time < B.time) return -1;
    return 0;
}
void pickProblem(PickOrder **pick_order, TokenHash* mail_hash, Data* data){
    int mailN = data->n_mails, queryN = data->n_queries;
    for (int i = 0; i < queryN; i++)
        pick_order[i]->id = i;
    long long int prefix[mailN];
    long long int log_prefix[mailN];
    int sorted[mailN];
    compute_prefix(mailN, mail_hash->len, sorted, log_prefix, prefix);
    for (int i = 0; i < queryN; i++){
        if (data->queries[i].type == expression_match){
            int expr_token = strlen((data->queries[i]).data.expression_match_data.expression) / 3;
            pick_order[i]->time = expr_token * log_prefix[mailN-1] / (data->queries[i]).reward;
        }   
        if (data->queries[i].type == find_similar){
            int l = -1, r = mailN;
            int tokenI = (data->queries[i]).data.find_similar_data.mid;
            int token = mail_hash->len[tokenI];
            while(l < r-1){
                int m = (l+r) / 2;
                if (token <= sorted[m]) r = m;
                else l = m;
            }
            int sortedI = r;
            int timecomp = prefix[sortedI-1]*log(token) + token*(log_prefix[mailN-1]-log_prefix[sortedI]);
            pick_order[i]->time = timecomp / (data->queries[i]).reward;
        }
        else{
            int len = (data->queries[i]).data.group_analyse_data.len;
            pick_order[i]->time = len*log(len) / (data->queries[i]).reward;
        }
    }
    qsort(pick_order, queryN, sizeof(PickOrder), pickComp);
 
}


