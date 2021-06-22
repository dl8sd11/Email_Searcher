main: main.c api.h src/*.h
	gcc main.c -o main -O3 -std=c11 -w -lm -g

main-gen: main.c api.h src/*.h
	gcc main-gen.c -o main-gen -O3 -std=c11 -w -lm

validator/validator: validator/validator.cpp
	g++ validator/validator.cpp -o validator/validator -O3 

debug: main.c api.h src/*.h
	gcc main.c -o debug -O3 -std=c11 -w -lm -g -fsanitize=address -fsanitize=undefined -DDBG

.PHONY=run
run: main validator/validator
	./main < testdata/test.in | validator/validator

sub: main-gen validator/validator
	./main-gen < testdata/test.in | validator/validator

test: debug validator/validator
	./debug < testdata/test.in | validator/validator

