CC = clang++
FLAGS = -g -std=c++17 -stdlib=libstdc++

CPP4Tokenizer = test/tokenizer-tester.cpp \
				src/token.cpp \
				src/tokenizer.cpp

CPP4Parser = test/parser-tester.cpp \
			 src/token.cpp \
			 src/tokenizer.cpp \
			 src/parser.cpp \
			 src/codegen.cpp

test: tmp/tokenizer-tester\
	  tmp/parser-tester
	tmp/tokenizer-tester
	tmp/parser-tester

tmp/tokenizer-tester: ${CPP4Tokenizer} | tmp
	${CC} ${FLAGS} $^ -o $@

tmp/parser-tester: ${CPP4Parser}
	${CC} ${FLAGS} $^ -o $@

tmp:
	mkdir tmp

.PHONY: clean
clean:
	rm -r ./tmp
