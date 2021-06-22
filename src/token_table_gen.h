#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "io.h"
#include "helper.h"

int reorderMailHash (TokenHash* mail_hash, int n_mails) {
  // step 1 : remap mail_hash.hash
  int allToken = 0;
  for (int i = 0; i < n_mails; i++){
    allToken += mail_hash->len[i];
  }
  HASH_TYPE* allTokenHash = (HASH_TYPE*)malloc(sizeof(HASH_TYPE) * allToken);
  int id = 0;
  for (int i = 0; i < n_mails; i++){
    for (int j = 0; j < mail_hash->len[i]; j++)
      allTokenHash[id++] = mail_hash->hash[i][j];
  }
  qsort(allTokenHash, allToken, sizeof(HASH_TYPE), hashComp);
  int uniqueToken = 0;
  uniqueHash(allTokenHash, allToken, &uniqueToken);
  for (int i = 0; i < n_mails; i++){
    for (int j = 0; j < mail_hash->len[i]; j++)
      mail_hash->hash[i][j] = SearchArrayId(allTokenHash, uniqueToken, mail_hash->hash[i][j]);
  }

  free(allTokenHash);
  // *allTokenHash : array storing unique hashes
  // uniqueToken : length of the allTokenHash
  // step 1 end

  return uniqueToken;
}
     
TokenTable* genTokenTable(TokenHash* mail_hash, int mail_count, int hash_len) {
  int* precount = (int*) calloc(hash_len, sizeof(int));
  for (int i=0; i<mail_count; i++) {
    for (int j=0; j<mail_hash->len[i]; j++) {
      precount[(int)mail_hash->hash[i][j]]++;
    }
  }
  int** tokenRef = (int**) malloc(sizeof(int*)*hash_len);                                                                                                       
  TokenTable* result = (TokenTable*)malloc(sizeof(TokenTable));
  result->len = (int*)malloc(sizeof(int) * hash_len);

  result->mailId = tokenRef;
  for (int i=0; i<hash_len; i++) {
    if(precount[i]) { 
      tokenRef[i] = (int*) malloc(sizeof(int)*precount[i]);
    } else {
      tokenRef[i] = NULL;
    }
    result->len[i] = precount[i];
  }
  for (int i=0; i<mail_count; i++) {
    for (int j=0; j<mail_hash->len[i]; j++) {
      int num = (int) mail_hash->hash[i][j];
      tokenRef[num][precount[num]-1] = i;
      precount[num]--;
    }
  }
  free(precount);
  return result;
}
