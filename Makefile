main: main.c api.h
	gcc main.c -o main -std=c11 -w -g -fsanitize=address -fsanitize=undefined

validator/validator: validator/validator.cpp
	g++ validator/validator.cpp -o validator/validator -O3 

.PHONY=run
run: main validator/validator
	./main < testdata/test.in | validator/validator
