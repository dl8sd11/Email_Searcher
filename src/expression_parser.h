#pragma once

#include "io.h"
bool ckeck(char* token, int* mail_hash, int mail_len);
bool isParenthese(char key);
bool isOperator(char key);
bool isNot(char key);
bool inToken(char key);
int* Parse(int stidx, char* input, int* mail_hash, int mail_len);
bool expression_parser(char* expression, int* mail_hash, int mail_len);

