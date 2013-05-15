SRC=src
OBJ=obj
ASM=asm

default: lc3sim program test

lc3sim:
	gcc -g -o $(OBJ)/lc3sim -lncurses $(SRC)/* 

program:
	as2obj $(ASM)/program.asm $(OBJ)/program

test:
	$(OBJ)/lc3sim $(OBJ)/program.obj

clean:
	rm $(OBJ)/*
