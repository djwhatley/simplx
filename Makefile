SRC=src
OBJ=obj
ASM=asm

default: simplx program test

#lc3sim:
#	gcc -g -o $(OBJ)/lc3sim -lncurses $(SRC)/* 

simplx:
	gcc -g -o $(OBJ)/simplx -lncurses $(SRC)/*

program:
	as2obj $(ASM)/program.asm $(OBJ)/program

test:
	$(OBJ)/simplx $(OBJ)/program.obj

#old:
#	$(OBJ)/lc3sim $(OBJ)/program.obj

clean:
	rm $(OBJ)/*
