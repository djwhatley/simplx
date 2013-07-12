SRC=src
OBJ=bin
ASM=asm

default: simplx program

#lc3sim:
#	gcc -g -o $(OBJ)/lc3sim -lncurses $(SRC)/* 

simplx:
	gcc -g -o $(OBJ)/simplx -lncurses $(SRC)/*

program:
	as2obj $(ASM)/program.asm

test:
	$(OBJ)/simplx $(ASM)/program.obj

#old:
#	$(OBJ)/lc3sim $(OBJ)/program.obj

clean:
	rm $(OBJ)/*
