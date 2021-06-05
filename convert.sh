#!/bin/bash

sed 's@#include "api.h"@// apiapiapi@g' main.c > main-api.c
sed -i 's@#include "../api.h"@// pppppp@g' src/io.h
quom main-api.c gen.c
sed -i 's@// pppppp@#include "../api.h"@g' src/io.h
sed 's@// apiapiapi@#include "api.h"@g' gen.c > main-gen.c
rm main-api.c
