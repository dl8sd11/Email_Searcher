#pragma once
#include "io.h"
#include "hash.h"
#include <string.h>
#define HASH_TYPE int
HASH_TYPE* token_parser(char* content, int* mail_token_len);
//int* token_parser_hash(char *content);
TokenHash* mail_parser (Data* data);
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "helper.h"


HASH_TYPE* token_parser (char* content, int* mail_token_len) {
  int N = strlen(content);
  (*mail_token_len) = 0;
  for (int i=0; i<N; i++) {
    if (valid_char(content[i]) && (i == 0 || !valid_char(content[i-1]))) {
      (*mail_token_len)++;
    }
  }

  HASH_TYPE* substringSet = (HASH_TYPE*)malloc(sizeof(HASH_TYPE*)*(*mail_token_len));		//the set of all sliced token
  int substringIdx = 0;
  char buf[100];


  int bid = 0;
  for (int i=0; i<N + 1; i++) {
    if (i < N && valid_char(content[i])) {
      buf[bid++] = lower(content[i]);
    } 
    if ((i == N || !valid_char(content[i])) && (i && valid_char(content[i-1]))) {
      char* cur = (char*)malloc(sizeof(char) * (bid + 1));
      buf[bid] = 0;
      strcpy(cur, buf);
      substringSet[substringIdx++] = hash1(cur);
      bid = 0;
    }
  }

  return substringSet;
}

TokenHash* mail_parser(Data* data) {
  TokenHash* mail_hashes = (TokenHash*)malloc(sizeof(TokenHash));
  mail_hashes->hash = (HASH_TYPE**)malloc(sizeof(HASH_TYPE*)*data->n_mails);
  mail_hashes->len = (int*)malloc(sizeof(int)*data->n_mails);
  int* mail_len = (int*) malloc(sizeof(int));
  char c[100305];
  for (int i=0; i<data->n_mails; i++) {
    c[0] = 0;
    strcat(c, data->mails[i].content);
    strcat(c, data->mails[i].subject);

    HASH_TYPE* token_hashes = token_parser(c, mail_len);
    qsort(token_hashes, *(mail_len), sizeof(HASH_TYPE), hashComp);

    int unq_len = 0;
    uniqueHash(token_hashes, *(mail_len), &unq_len);
    mail_hashes->hash[i] = token_hashes;
    mail_hashes->len[i] = unq_len;

  }
  free(mail_len);
  return mail_hashes;
}
