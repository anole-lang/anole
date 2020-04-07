ADD =

CC = clang++
FLAGS = -std=c++17 -stdlib=libstdc++ -ldl -lstdc++fs -lreadline ${ADD}

DIR_TMP = tmp

OBJ = tmp/tokenizer.so tmp/parser.so tmp/code.so tmp/objects.so tmp/context.so tmp/repl.so
FPOBJ = $(addprefix $(shell pwd)/, ${OBJ})

test: tmp/tokenizer-tester \
	  tmp/parser-tester \
	  tmp/sample-tester
	tmp/tokenizer-tester
	tmp/parser-tester
	tmp/sample-tester

tmp/tokenizer.so: src/token.cpp \
				  src/tokenizer.cpp | ${DIR_TMP}
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

tmp/tokenizer-tester: test/tokenizer-tester.cpp ${OBJ} | ${DIR_TMP}
	${CC} ${FLAGS} $< ${FPOBJ} -o $@

tmp/parser-tester: test/parser-tester.cpp ${OBJ}
	${CC} ${FLAGS} $< ${FPOBJ} -o $@

tmp/sample-tester: test/sample-tester.cpp ${OBJ}
	${CC} ${FLAGS} $< ${FPOBJ} -o $@

${DIR_TMP}:
	mkdir $@

.PHONY: clean
clean:
	rm -rf ${DIR_TMP}
