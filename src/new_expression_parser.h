#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "hash.h"
#include "io.h"

typedef struct llistNode {
  char type;
  bool rev;
  int hash;
  char oprType;
  char prtType;
  struct llistNode* next;
} node;

node* Parse(char* input);
bool check(int token_hash, int* mail_hash, int mail_len);
int* evalChecker(node* parseHead, int* mail_hash, int mail_len, bool inPar);
bool expression_parser(char* expression, int* mail_hash, int mail_len);

bool token_cont(char c) {
  if (c >= 'A' && c <= 'Z') return true;
  if (c >= 'a' && c <= 'z') return true;
  if (c >= '0' && c <= '9') return true;
  return false;
}


node* Parse(char* input) {
  int i = 0;
  bool inToken = false;
  bool unput_rev = false;
  node *head, *res;
  bool headAssigned = false;
  while (input[i] != 0) {
    //printf("here at %c\n", input[i]);
    if (input[i] == '(') {
      node* newNode = (node*) malloc(sizeof(node));
      if (!headAssigned) {
        head = newNode;
        res = head;
        headAssigned = true;
      } else {
        res->next = newNode;
        res = res->next;
      }
      if (token_cont(input[i+1])) {
        i++;
        char token[50];
        int tokenIdx = 0;
        while (input[i] != ')') {
          token[tokenIdx] = input[i];
          tokenIdx++;
          i++;
        }
        token[tokenIdx] = '\0';
        //printf("Caught token: %s\n", token);
        int hashResult = hash1(token);
        if (unput_rev) {
          newNode->rev = true;
          unput_rev = false;
        } else {
          newNode->rev = false;
        }
        newNode->type = 't';
        newNode->hash = hashResult;
        newNode->next = NULL;
        i++;
      } else {
        if (unput_rev) {
          newNode->rev = true;
          unput_rev = false;
        } else {
          newNode->rev = false;
        }
        newNode->type = 'p';
        newNode->prtType = '(';
        i++;
      }
    } else if (input[i] == ')') {
      node* newNode = (node*) malloc(sizeof(node));
      res->next = newNode;
      res = res->next;
      newNode->type = 'p';
      newNode->prtType = ')';
      newNode->next = NULL;
      i++;
    } else if (input[i] == '!') {
      unput_rev = true;
      i++;
    } else if (token_cont(input[i])) {
      node* newNode = (node*) malloc(sizeof(node));
      char token[50];
      int tokenIdx = 0;
      while (token_cont(input[i])) {
        token[tokenIdx] = input[i];
        tokenIdx++;
        i++;
      }
      token[tokenIdx] = '\0';
      int hashRes = hash1(token);
      //printf("Caught token: %s\n", token);
      res->next = newNode;
      res = res->next;
      newNode->type = 'h';
      newNode->hash = hashRes;
      if (unput_rev) {
        newNode->rev = true;
        unput_rev = false;
      } else {
        newNode->rev = false;
      }
      newNode->next = NULL;
    } else {
      node* newNode = (node*) malloc(sizeof(node));
      res->next = newNode;
      res = res->next;
      newNode->type = 'o';
      newNode->oprType = input[i];
      i++;
    }
  }
  return head;
}

bool check(int token_hash, int* mail_hash, int mail_len) {
    //int token_hash = hash1(token);
    // mail_hash should be sorted

    int L = -1, R = mail_len;
    // lower_bound(mail_hash, token_hash)
    while (L < R - 1) {
        int M = (L + R) >> 1;
        if (mail_hash[M] <= token_hash) L = M;
        else R = M;
    }

    //printf("%d %d\n", token_hash, L != -1 && mail_hash[L] == token_hash);
    return L != -1 && mail_hash[L] == token_hash;
}

int* evalChecker(node *head, int *mail_hash, int mail_len, bool inPar) {
  int resSum[1000], resIdx = 0;
  int* output = (int*) malloc(sizeof(int)*2);
  bool mult = false;
  int passedNode = 0;
  while (head != NULL) {
    if (head->type == 'p') {
      if (head->prtType == '(') {
        int* inside = evalChecker(head->next, mail_hash, mail_len, true);
        int val = inside[0];
        if (head->rev) {
          if (val) val = 0;
          else val = 1;
        }
        if (!mult) resSum[resIdx] = val;
        else resSum[resIdx] *= val;
        for (int i=0; i<=inside[1]+1; i++) head = head->next;
        passedNode += inside[1]+2;
        free(inside);
      } else {
        int sum = 0;
        for (int i=0; i<=resIdx; i++) sum += resSum[i];
        if (sum) output[0] = 1;
        else output[0] = 0;
        output[1] = passedNode;
        return output;
      }
    } else if (head->type == 'o') {
      if (head->oprType == '&') {
        head = head->next;
        passedNode++;
        mult = true;
      } else {
        head = head->next;
        resIdx++;
        mult = false;
        passedNode++;
      }
    } else {
      int val = (int) check(head->hash, mail_hash, mail_len);
      if (head->rev) {
        if (val) val = 0;
        else val = 1;
      }
      if (mult) resSum[resIdx] *= val;
      else resSum[resIdx] = val;
      head = head->next;
      passedNode++;
    }
  }
  int sum = 0;
  for (int i=0; i<=resIdx; i++) sum += resSum[i];
  output[0] = sum;
  output[1] = passedNode;
  return output;
}
        
bool expression_parser(char *expression, int *mail_hash, int mail_len) {
  node* parseResult = Parse(expression);
  int* matchResult = evalChecker(parseResult, mail_hash, mail_len, false);
  while (parseResult != NULL) {
    node* temp = parseResult;
    parseResult = parseResult->next;
    free(temp);
  }
  bool result = (bool) matchResult[0];
  //printf("%s\n", expression);
  //printf(">>final result = %d<<\n", (int) result);
  free(matchResult);
  return result;
}


