ADD =

CC = clang++
FLAGS = -std=c++17 -stdlib=libstdc++ ${ADD}
LDS = -ldl -lstdc++fs -lreadline

DIR_TMP = tmp

OBJ = tmp/error.so tmp/objects.so tmp/runtime.so tmp/compiler.so
FPOBJ = $(addprefix $(shell pwd)/, ${OBJ})

test: tmp/test
	tmp/test

tmp/error.so: anole/error.cpp | ${DIR_TMP}
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

tmp/objects.so: anole/objects/*.cpp
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

tmp/runtime.so: anole/runtime/*.cpp
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

tmp/compiler.so: anole/compiler/*.cpp
	${CC} ${FLAGS} $^ -shared -fPIC -o $@

tmp/test: test/test.cpp ${OBJ}
	${CC} ${FLAGS} $< ${FPOBJ} ${LDS} -lgtest -lpthread -o $@

${DIR_TMP}:
	mkdir $@

.PHONY: clean
clean:
	rm -rf ${DIR_TMP}
