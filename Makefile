ADD =

CC = clang++
FLAGS = -std=c++17 -stdlib=libstdc++ ${ADD}

DIR_OBJ = obj
DIR_BIN = bin

OBJ = obj/tokenizer.so obj/parser.so obj/code.so obj/objects.so obj/context.so obj/repl.so
FPOBJ = $(addprefix $(shell pwd)/, ${OBJ})

test: tmp/tokenizer-tester \
	  tmp/parser-tester \
	  tmp/sample-tester
	tmp/tokenizer-tester
	tmp/parser-tester
	tmp/sample-tester

obj/tokenizer.so: src/token.cpp \
				  src/tokenizer.cpp | ${DIR_OBJ}
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

obj/parser.so: src/parser.cpp
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

obj/code.so: src/codegen.cpp \
			 src/code.cpp
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

obj/objects.so: src/object.cpp \
				src/noneobject.cpp \
				src/boolobject.cpp \
				src/listobject.cpp \
				src/dictobject.cpp \
				src/funcobject.cpp \
				src/contobject.cpp \
				src/floatobject.cpp \
				src/stringobject.cpp \
				src/integerobject.cpp \
				src/builtinfuncobject.cpp
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

obj/context.so: src/context.cpp \
				src/builtins.cpp
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

obj/repl.so: src/repl.cpp
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

tmp/tokenizer-tester: test/tokenizer-tester.cpp ${OBJ} | ./tmp
	${CC} ${FLAGS} $< ${FPOBJ} -o $@

tmp/parser-tester: test/parser-tester.cpp ${OBJ}
	${CC} ${FLAGS} $< ${FPOBJ} -o $@

tmp/sample-tester: test/sample-tester.cpp ${OBJ}
	${CC} ${FLAGS} $< ${FPOBJ} -o $@

${DIR_OBJ}:
	mkdir $@

./tmp:
	mkdir tmp

.PHONY: clean
clean:
	rm -rf ${DIR_OBJ}
	rm -rf ./tmp
