# BIN = bin
# LIB = lib
# INCLUDE = include

# OBJS = main.cpp src/*.cpp

# CC = g++

# COMPILER_FLAGS = -w -Wall -ggdb3

# OBJ_NAME = main

# all : $(OBJS)
# 	$(CC) $(OBJS) $(COMPILER_FLAGS) -I$(INCLUDE) -L$(LIB) -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -o $(BIN)/$(OBJ_NAME)
all:
	cd bin; ninja -j4
	.\bin\Defblade