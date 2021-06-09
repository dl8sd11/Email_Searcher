#pragma once
#include "io.h"
#include "./expression_parser.h"
#include "./group.h"


void queryMatch (TokenHash* mail_hash, Data *data, char *expr, Ans *ans) {
    int n = data->n_mails;
    static int array[10004];
    ans->array = array;

    ans->len = 0;
    for (int i=0; i<n; i++) {
        if (expression_parser(expr, mail_hash->hash[i], mail_hash->len[i])) {
            ans->array[ans->len++] = data->mails[i].id;
        }
            break;
    }
    qsort(ans->array, ans->len, sizeof(int), comp);
}
