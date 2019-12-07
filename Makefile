TARGET = ice

ADD =

CC = clang++
FLAGS = -g -std=c++17 -stdlib=libstdc++ ${ADD}

DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_BIN = ./bin

SRC = $(wildcard ${DIR_SRC}/*.cpp)
OBJ = $(patsubst %.cpp, ${DIR_OBJ}/%.o, $(notdir ${SRC}))

# for ice

BIN_TARGET = $(DIR_BIN)/$(TARGET)

${BIN_TARGET}: ${OBJ} | ${DIR_BIN}
	${CC} ${FLAGS} ${OBJ} -o $@

${DIR_OBJ}/%.o: ${DIR_SRC}/%.cpp | ${DIR_OBJ}
	${CC} ${FLAGS} -c $< -o $@

${DIR_BIN}:
	mkdir $@

${DIR_OBJ}:
	mkdir $@

# for testers

CPP4Tokenizer = test/tokenizer-tester.cpp \
			src/token.cpp \
			src/tokenizer.cpp

CPP4Parser = test/parser-tester.cpp \
			 src/token.cpp \
			 src/tokenizer.cpp \
			 src/parser.cpp \
			 src/codegen.cpp

CPP4Frame = test/frame-tester.cpp \
			src/token.cpp \
			src/tokenizer.cpp \
			src/parser.cpp \
			src/codegen.cpp \
			src/frame.cpp \
			src/object.cpp

tmp/tokenizer-tester: ${CPP4Tokenizer} | ./tmp
	${CC} ${FLAGS} $^ -o $@

tmp/parser-tester: ${CPP4Parser}
	${CC} ${FLAGS} $^ -o $@

tmp/frame-tester: ${CPP4Frame}
	${CC} ${FLAGS} $^ -o $@

test: tmp/tokenizer-tester \
	  tmp/parser-tester \
	  tmp/frame-tester
	tmp/tokenizer-tester
	tmp/parser-tester
	tmp/frame-tester

./tmp:
	mkdir tmp

.PHONY: clean
clean:
	rm -rf ${DIR_BIN} ${DIR_OBJ}
	rm -rf ./tmp
