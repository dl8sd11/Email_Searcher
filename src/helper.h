#pragma once
#define HASH_TYPE long long
#include "io.h"
#include <string.h>

bool valid_char(char k) {								//check if character in [A-Za-z0-9]
  if (k > 47 && k < 58) return true;
  if (k > 64 && k < 91) return true;
  if (k > 96 && k < 123) return true;
  return false;
}

int hashComp(const void *a, const void *b) {
  HASH_TYPE A = *(HASH_TYPE *)a, B = *(HASH_TYPE *)b;
  if (A > B) return 1;
  if (A < B) return -1;
  return 0;
}

int pickComp(const void *a, const void *b){
  PickOrder A = *(PickOrder *)a, B = *(PickOrder *)b;
  if (A.time > B.time) return 1;
  if (A.time < B.time) return -1;
  return 0;
}

int groupComp (const void *a, const void *b) {
  return ((SimilarGroup*)a)->score > ((SimilarGroup*)b)->score ? -1 : 1;
}

int jacComp (const void *a, const void *b) {
  return ((SimilarData*)b)->jaccard - ((SimilarData*)a)->jaccard > 0 ? 1 : -1;
}

int mailIdComp (const void *a, const void *b) {
  return ((mail*)a)->id - ((mail*)b)->id;
}

/*
unique hashes 
input: a sorted array of hashes
output: uniqued array in hashes[0..resN]
*/
void uniqueHash (HASH_TYPE* hashes, int len, int* resN) {
  for (int i=0, st=0; i<len; i++) {
    if (i == len-1 || hashes[i+1] != hashes[st]) {
      hashes[(*resN)++] = hashes[st];
      st = i + 1;
    }
  }
}

/*
binary search hash in array 
output: 0, 1
*/
int hashInSortedArray (HASH_TYPE hash, HASH_TYPE* array, int sz) {
  int L = -1, R = sz;
  while (L < R - 1) {
    int M = (L + R) >> 1;
    if (array[M] < hash) L = M;
    else R = M;
  }
  return R < sz && array[R] == hash;
}
int SearchArrayId(int *array, int arrayN, int target){
	int L = -1, R = arrayN, M;
	while (L < R-1){
		M = (L+R)/2;
		if (array[M] >= target) R = M;
		else L = M;
	}
	return R;
}

char lower (char c) {
  if (c >= 'A' && c <= 'Z') return c - 'A' + 'a';
  return c;
}

void push_back (int **array, int item, int* cap, int *sz) {
  if (*sz == *cap) {
  (*cap) *= 2;
  *array = (int*)realloc(*array, (*cap)*sizeof(int));
  }
  (*array)[(*sz)++] = item;
}

