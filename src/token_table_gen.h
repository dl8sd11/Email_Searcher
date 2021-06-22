#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include "io.h"
     
int** genTokenTable(TokenHash* mail_hash, int mail_count, int hash_len) {
  int* precount = (int*) calloc(hash_len, sizeof(int));
  for (int i=0; i<mail_count; i++) {
    for (int j=0; j<mail_hash->len[i]; j++) {
      precount[(int)mail_hash->hash[i][j]]++;
    }
  }
  int** tokenRef = (int**) malloc(sizeof(int*)*hash_len);                                                                                                       
  for (int i=0; i<hash_len; i++) {
    if(precount[i]) { 
      tokenRef[i] = (int*) malloc(sizeof(int)*precount[i]);
    } else {
      tokenRef[i] = NULL;
    }
  }
  for (int i=0; i<mail_count; i++) {
    for (int j=0; j<mail_hash->len[i]; j++) {
      int num = (int) mail_hash->hash[i][j];
      tokenRef[num][precount[num]-1] = i;
      precount[num]--;
    }
  }
  free(precount);
  return tokenRef;
}
