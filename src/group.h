#pragma once
#include "io.h"
#include <stdlib.h>
#include "./hash.h"

int comp(const void *a, const void *b) {
    int A = *(int *)a, B = *(int *)b;
    if (A > B) return 1;
    if (A < B) return -1;
    return 0;
}

void unique (int* hashes, int len, int* resN) {
    for (int i=0, st=0; i<len; i++) {
        if (i == len-1 || hashes[i+1] != hashes[st]) {
            hashes[(*resN)++] = hashes[st];
            st = i + 1;
        }
    }
}

int getId (char *name, int* hashes, int hashN) {
    int nameHash = hash1(name);

    // lower_bound(nameHash)
    int L = -1, R = hashN;
    while (L < R - 1) {
        int M = (L + R) >> 1;
        if (hashes[M] >= nameHash) R = M;
        else L = M;
    }
    return R;
}

int fnd (int x, int *djs) {
    if (x == djs[x]) return x;
    djs[x] = fnd(djs[x], djs);
    return djs[x];
}

void merge (int a, int b, int *djs, int *sz) {
    a = fnd(a, djs);
    b = fnd(b, djs);
    if (a == b) return;
    if (sz[a] > sz[b]) {
        int tmp = a;
        a = b;
        b = tmp;
    }

    djs[a] = b;
    sz[b] += sz[a];
}

void queryGroup (Data *data, Ans *ans, int len, int *mids) {

    int* hashes = (int*)malloc(len * 2 * sizeof(int));
    for (int i=0; i<len; i++) {
        int mid = mids[i];
        hashes[i<<1] = hash1(data->mails[mid].from);
        hashes[i<<1|1] = hash1(data->mails[mid].to);
        //fprintf(stderr, "%s %s\n", data->mails[mid].from, data->mails[mid].to);
    }

    qsort(hashes, len*2, sizeof(int), comp);
    int hashN = 0;
    unique(hashes, len*2, &hashN);

    int* djs = (int*)malloc(hashN * sizeof(int));
    int* sz = (int*)malloc(hashN * sizeof(int));
    for (int i=0; i<hashN; i++) {
        djs[i] = i;
        sz[i] = 1;
    }

    for (int i=0; i<len; i++) {
        int mid = mids[i];
        int fid = getId(data->mails[mid].from, hashes, hashN);
        int tid = getId(data->mails[mid].to, hashes, hashN);
        //fprintf(stderr, "%d %d\n", fid, tid);
        merge(fid, tid, djs, sz);
    }

    ans->len = 2;
    ans->array = (int*)malloc(2 * sizeof(int));
    ans->array[0] = 0;
    ans->array[1] = 0;

    for (int i=0; i<hashN; i++) {
        if (djs[i] == i && sz[i] > 1) {
            ans->array[0]++;
            //fprintf(stderr, "group %d %d\n", djs[i], sz[i]);
            if (sz[i] > ans->array[1]) {
                ans->array[1] = sz[i];
            }
        }
    }
    //fprintf(stderr, "%d %d\n",ans->array[0], ans->array[1]);
    free(djs);
    free(sz);
    free(hashes);

}
