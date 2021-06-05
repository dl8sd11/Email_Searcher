#pragma once
#include "io.h"
#include "hash.h"
#include "group.h"
char** token_parser(char* content, int* mail_token_len);
//int* token_parser_hash(char *content);
TokenHash* mail_parser (Data* data);
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool valid_char(char k) {								//check if character in [A-Za-z0-9]
	if (k > 47 && k < 58) return true;
	if (k > 64 && k < 91) return true;
	if (k > 96 && k < 123) return true;
	return false;
}

char** token_parser(char* content, int* mail_token_len) {
	int originContentIdx = 0;							//index of the content
	int substringIdx = 0;								//index of each substring(reset to 0 when start another one)	
	char** substringSet = (char**)malloc(sizeof(char*)*50);		//the set of all sliced token
	char* substring = (char*)malloc(sizeof(char)*4);			//the first token
	int substringMaxLen = 4;							//initalize the size of token
	int substringSetIdx = 0;							//index of the token set
	int substringSetSize = 50;							//initalize the size of token set

	while (content[originContentIdx] != 0) {			//start to read the content
		char key = content[originContentIdx];			//key = current character
		bool cut = false;								//cut shows the state 1.in a token -> false 2.out of a token -> true
		if (cut) {										//if currently out of a token
			if (valid_char(key)) {						//if valid character read
				cut = false;							//turn cut off
				substring = (char*)malloc(sizeof(char)*4);		//allocate a new space for token
				substringIdx = 0;						//reset index inside token
				substring[substringIdx] = key;			//put key into the stored token
				substringIdx++;							//move to next index
				originContentIdx++;						//move to next character(key)
			} else {									//if key still invalid
				originContentIdx++;						//move to next character
			}
		} else {										//if currently in a token
			if (valid_char(key)) {						//if valid character read
				if (substringIdx < substringMaxLen-1) { //if the token is not about to be full
					substring[substringIdx] = key;		//fill in the key
					substringIdx++;						//move to next index
					originContentIdx++;					//move to next character(key)
				} else {								//if the token is almost full
					substringMaxLen *= 2;				//resize the token 
					substring = (char*)realloc(substring, sizeof(char)*substringMaxLen);	//allocate it into double quota
					substring[substringIdx] = key;		//fill in the key
					substringIdx++;						//move to next index
					originContentIdx++;					//move to next character(key)
				}
			} else {									//if an invalid key read
				if (substringIdx == 0 && substringSetIdx == 0) {	//handle if the first token not yet started
					originContentIdx++;
					continue;
				}
				cut = true;								//cut the token
				substring[substringIdx] = '\0';			//put end of string in the token
				substringMaxLen = 4;					//reset the token size
				substringIdx = 0;						//reset the token index
				substringSet[substringSetIdx] = substring;	//put the token into the token set
				substringSetIdx++;						//move to the next index of token set
				if (!(substringSetIdx < substringSetSize-1)) {	//if the token set is almost full
					substringSetSize *= 2;				//resize the token set
					substringSet = (char**)realloc(substringSet, sizeof(char*)*substringSetSize);	//allocate the token set into double quota
				}
				originContentIdx++;						//move to next character(key)
			}
		}

		if (!cut) {										//if content end with a valid character (still in a token)
			substring[substringIdx] = '\0';				//put end of string in the token (cut the token)
			substringSet[substringSetSize] = substring;	//put the token into the token set
		}
	}
	*(mail_token_len) = substringSetIdx;
	return substringSet;							//return the token set
}

TokenHash* mail_parser(Data* data) {
	TokenHash* mail_hashes = (TokenHash*)malloc(sizeof(TokenHash));
	mail_hashes->hash = (int**)malloc(sizeof(int*)*data->n_mails);
	mail_hashes->len = (int*)malloc(sizeof(int)*data->n_mails);
	int* mail_len = (int*) malloc(sizeof(int));
	for (int i=0; i<data->n_mails; i++) {
		char** tokens = token_parser(data->mails[i].content, mail_len);
		int* token_hashes = (int*) malloc(sizeof(int)*(*(mail_len)));
		for (int j=0; j<*(mail_len); j++) {
			token_hashes[j] = hash1(tokens[j]);
			free(tokens[j]);
		}
		qsort(token_hashes, *(mail_len), sizeof(int), comp);
		int unq_len = 0;
		unique(token_hashes, *(mail_len), &unq_len);
		mail_hashes->hash[i] = token_hashes;
		mail_hashes->len[i] = unq_len;
		free(tokens);
	}
	free(mail_len);
	return mail_hashes;
}
