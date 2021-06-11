#pragma once
#define HASH_TYPE long long
int compHash(const void *a, const void *b) {
    HASH_TYPE A = *(HASH_TYPE *)a, B = *(HASH_TYPE *)b;
    if (A > B) return 1;
    if (A < B) return -1;
    return 0;
}

void uniqueHash (HASH_TYPE* hashes, int len, int* resN) {
    for (int i=0, st=0; i<len; i++) {
        if (i == len-1 || hashes[i+1] != hashes[st]) {
            hashes[(*resN)++] = hashes[st];
            st = i + 1;
        }
    }
}
