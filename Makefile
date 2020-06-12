ADD =

CC = clang++
FLAGS = -std=c++17 -stdlib=libstdc++ -ldl -lstdc++fs -lreadline ${ADD}

DIR_TMP = tmp

OBJ = tmp/error.so tmp/tokenizer.so tmp/parser.so tmp/code.so tmp/objects.so tmp/context.so tmp/repl.so
FPOBJ = $(addprefix $(shell pwd)/, ${OBJ})

test: tmp/test
	tmp/test

tmp/error.so: src/error.cpp | ${DIR_TMP}
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

tmp/tokenizer.so: src/token.cpp \
				  src/tokenizer.cpp
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

tmp/parser.so: src/parser.cpp
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

tmp/code.so: src/codegen.cpp \
			 src/code.cpp
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

tmp/objects.so: src/object.cpp \
				src/noneobject.cpp \
				src/boolobject.cpp \
				src/listobject.cpp \
				src/enumobject.cpp \
				src/dictobject.cpp \
				src/funcobject.cpp \
				src/contobject.cpp \
				src/floatobject.cpp \
				src/moduleobject.cpp \
				src/stringobject.cpp \
				src/integerobject.cpp \
				src/builtinfuncobject.cpp
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

tmp/context.so: src/context.cpp \
				src/builtins.cpp
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

tmp/repl.so: src/repl.cpp
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

tmp/test: test/test.cpp ${OBJ} | ${DIR_TMP}
	${CC} ${FLAGS} $< ${FPOBJ} -lgtest -lpthread -o $@

${DIR_TMP}:
	mkdir $@

.PHONY: clean
clean:
	rm -rf ${DIR_TMP}
