#!/bin/bash

sed 's@#include "api.h"@// apiapiapi@g' main.c src/hash.c src/token_parser.c src/expression_parser.c> main-api.c
quom main-api.c gen.c
sed 's@// apiapiapi@#include "api.h"@g' gen.c > main-gen.c
rm main-api.c
