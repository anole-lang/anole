TARGET = ice

ADD =

CC = clang++
FLAGS = -g -std=c++17 -stdlib=libstdc++

DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_BIN = ./bin

SRC = $(wildcard ${DIR_SRC}/*.cpp)
OBJ = $(patsubst %.cpp, ${DIR_OBJ}/%.o, $(notdir ${SRC}))

BIN_TARGET = $(DIR_BIN)/$(TARGET)

${BIN_TARGET}: ${OBJ} | ${DIR_BIN}
	${CC} ${FLAGS} ${OBJ} -o $@

${DIR_OBJ}/%.o: ${DIR_SRC}/%.cpp | ${DIR_OBJ}
	${CC} ${FLAGS} -c $< -o $@

${DIR_BIN}:
	mkdir $@

${DIR_OBJ}:
	mkdir $@

.PHONY: test clean

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

clean:
	rm -rf ${DIR_OBJ}/*.o
	rm -rf ./tmp
