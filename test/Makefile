CC = clang++
FLAGS = -g -std=c++17

test: tmp/tokenizer-tester\
	  tmp/parser-tester
	tmp/tokenizer-tester
	tmp/parser-tester

tmp/tokenizer-tester: tokenizer-tester.cpp \
					  ../src/token.cpp \
					  ../src/tokenizer.cpp \
					  | tmp
	${CC} ${FLAGS} $^ -o tmp/tokenizer-tester

tmp/parser-tester: parser-tester.cpp \
				   ../src/token.cpp \
				   ../src/tokenizer.cpp \
				   ../src/parser.cpp \
				   ../src/codegen.cpp \
				   ../src/operation.cpp
	${CC} ${FLAGS} $^ -o tmp/parser-tester

tmp:
	mkdir tmp

.PHONY: clean
clean:
	rm -r ./tmp
