CC=gcc
FILE=threadSA.c
FLAGS=-Wall -lpthread
OUT= -o SA

all:
	$(CC) $(FILE) $(FLAGS) $(OUT)

