#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "hash.h"
#include "io.h"

bool ckeck(char* token, int* mail_hash, int mail_len);
bool isParenthese(char key);
bool isOperator(char key);
bool isNot(char key);
bool inToken(char key);
int* Parse(int stidx, char* input, int* mail_hash, int mail_len);
bool expression_parser(char* expression, int* mail_hash, int mail_len);

bool check(char* token, int* mail_hash, int mail_len) {
    int token_hash = hash1(token);
    // mail_hash should be sorted

    int L = -1, R = mail_len;
    // lower_bound(mail_hash, token_hash)
    while (L < R - 1) {
        int M = (L + R) >> 1;
        if (mail_hash[M] <= token_hash) L = M;
        else R = M;
    }
    return L != -1 && mail_hash[L] == token_hash; 
}

bool inToken(char key) {
	if (key >= 'A' && key <= 'Z') return true;
	if (key >= 'a' && key <= 'z') return true;
	if (key >= '0' && key <= '9') return true;
	return false;
}

bool isParenthese(char key) {
	if (key == '(' || key == ')') return true;
	return false;
}

bool isNot(char key) {
	if (key == '!') return true;
	return false;
}

bool isOperator(char key) {
	if (key == '&' || key == '|') return true;
	return false;
}

int* Parse(int stidx, char* input, int* mail_hash, int mail_len) {
	int* output = (int*)malloc(sizeof(int)*2);
	bool parts[1000];
	int boolidx = 0;
	char token[100];
	int tokenidx = 0;
	bool mult = false;
	bool reverse = false;
	bool bv;
	int i = stidx;
	printf("im here\n");
	printf("%s\n", input);
	while (input[i] != 0) {
		//printf("at: %c\n", input[i]);
		if (inToken(input[i])) {
			//printf("in token\n");
			token[tokenidx] = input[i];
			tokenidx++;
			i++;
		} else if (isOperator(input[i])) {
			//printf("bv : %d\n", bv);
			if (reverse) {
				bv = !bv;
				reverse = false;
			}
			if (input[i] == '|') {
				if (mult) {
					parts[boolidx] = parts[boolidx] && bv;
					mult = false;
					boolidx++;
				} else {
					parts[boolidx] = bv;
					boolidx++;
				}
			} else {
				if (mult) parts[boolidx] = parts[boolidx] && bv;
				else {
					parts[boolidx] = bv;
					mult = true;
				}
			}
			i++;
		} else if (isNot(input[i])) {
			reverse = true;
			i++;
		} else if (isParenthese(input[i])) {
			if (input[i] == '(') {
				int* unlock;
				unlock = Parse(i+1, input, mail_hash, mail_len);
				i = unlock[0];
				bv = (bool)unlock[1];
				if (reverse) {
					bv = !bv;
					reverse = false;
				}
				if (mult) parts[boolidx] = parts[boolidx] && bv;
				else parts[boolidx] = bv;
				if (isOperator(input[i])) {
					printf("bv: %d opr: %c\n", bv, input[i]);
					if (input[i] == '|') {
						if (mult) {
							boolidx++;
							mult = false;
						} else {
							boolidx++;
						}
					} else {
						if (!mult)
							mult = true;
						}
					}
					i++;
			} else {
				i++;
				break;
			}
		}
	}
	if (tokenidx != 0) {
		token[tokenidx] = '\0';
		printf("token: %s\n", token);
		if (reverse) bv = !check(token, mail_hash, mail_len);
		else bv = check(token, mail_hash, mail_len);
		if (mult) parts[boolidx] = parts[boolidx] && bv;
		else parts[boolidx] = bv;
	}
	int k = 0;
	for (int j=0; j<= boolidx; j++) {
		k += (bool)parts[j];
	}
	printf("(exprs) returns %d\n", k);
	output[0] = i;
	output[1] = k;
	return output;
}

bool expression_parser(char* expression, int* mail_hash, int mail_len) {
	int* parseResult = Parse(0, expression, mail_hash, mail_len);
	bool ans = (bool)parseResult[1];
	free(parseResult);
  puts("done");
	return ans;
}
