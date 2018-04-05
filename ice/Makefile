DIR_INC = ./include
DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_BIN = ./bin

SRC = $(wildcard ${DIR_SRC}/*.cpp)
OBJ = $(patsubst %.cpp, ${DIR_OBJ}/%.o, $(notdir ${SRC})) 

TARGET = ice

BIN_TARGET = $(DIR_BIN)/$(TARGET)

CC = g++
CFLAGS = -g -Wall -std=c++11 -I$(DIR_INC)

CPPFLAGS = -std=c++11

${BIN_TARGET}:${OBJ}
	${CC} ${OBJ} -o $@

${DIR_OBJ}/%.o:${DIR_SRC}/%.cpp
	${CC} ${CFLAGS} -c $< -o $@


.PHONY: clean
clean:
	rm -rf ${DIR_OBJ}/*.o