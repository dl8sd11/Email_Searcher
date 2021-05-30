#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "expression_parser.h"

bool check(char* token) { return false; }	//fake checker function

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

int* Parse(int stidx, char* input) {
	int* output = malloc(sizeof(int)*2);
	bool parts[1000];
	int boolidx = 0;
	char token[100];
	int tokenidx = 0;
	bool and = false;
	bool not = false;
	int i = stidx;
	while (input[i] != 0) {
		if (inToken(input[i])) {
			token[tokenidx] = input[i];
			tokenidx++;
			i++;
		} else if (isOperator(input[i])) {
			token[tokenidx] = '\0';
			tokenidx = 0;
			bool bv = check(token);
			if (not) {
				bv = !bv;
				not = false;
			}
			if (input[i] == '|') {
				if (and) {
					parts[boolidx] = parts[boolidx] && bv;
					and = false;
					boolidx++;
				} else {
					parts[boolidx] = bv;
					boolidx++;
				}
			} else {
				if (and) parts[boolidx] = parts[boolidx] && bv;
				else {
					parts[boolidx] = bv;
					and = true;
				}
			}
			i++;
		} else if (isNot(input[i])) {
			not = true;
		} else if (isParenthese(input[i])) {
			if (input[i] == '(') {
				int* unlock;
				unlock = Parse(i+1, input);
				i = unlock[0];
				bool bv = (bool)unlock[1];
				if (not) {
					bv = !bv;
					not = false;
				}
				if (and) parts[boolidx] = parts[boolidx] && bv;
				else parts[boolidx] = bv;
				if (isOperator(input[i])) {
					if (input[i] == '|') {
						if (and) {
							boolidx++;
							and = false;
						} else {
							boolidx++;
						}
					} else {
						if (!and)
							and = true;
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
		bool bv;
		if (not) bv = !check(token);
		else bv = check(token);
		if (and) parts[boolidx] = parts[boolidx] && bv;
		else parts[boolidx] = bv;
	}
	int k = 0;
	for (int j=0; j<= boolidx; j++) {
		k += (bool)parts[j];
	}
	output[0] = i;
	output[1] = k;
	return output;
}

bool expression_parser(char* expression) {
	int* parseResult = Parse(0, expression);
	return (bool)parseResult[1];
}
