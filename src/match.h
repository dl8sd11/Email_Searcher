#pragma once
#include "io.h"
#include "./expression_parser.h"
#include "./group.h"
#include "./token_parser.h"


void queryMatch (TokenHash* mail_hash, Data *data, char *expr, Ans *ans) {
    int n = data->n_mails;
    static int array[10004];
    ans->array = array;

    ans->len = 0;

    int m = strlen(expr);
    for (int i=0; i<m; i++) {
      expr[i] = lower(expr[i]);
    }

    for (int i=0; i<n; i++) {
      if (data->mails[i].id != 8559) continue;
        if (expression_parser(expr, mail_hash->hash[i], mail_hash->len[i])) {
            ans->array[ans->len++] = data->mails[i].id;
        }
    }
    qsort(ans->array, ans->len, sizeof(int), comp);
}
