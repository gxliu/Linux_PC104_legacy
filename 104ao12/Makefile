#Makefile for 104-AO12-4

default: all

all: sample1 sample2 sample3 sample8
	@echo made all samples

sample1: sample1.c acceslib.c acceslib.h iogen.h
	cc -o sample1 sample1.c acceslib.c
	@echo sample1 made

sample2: sample2.c acceslib.c acceslib.h iogen.h
	cc -o sample2 sample2.c acceslib.c -lm
	@echo sample2 made

sample3: sample3.c acceslib.c acceslib.h iogen.h
	cc -o sample3 sample3.c acceslib.c
	@echo sample3 made

sample8: sample8.c acceslib.c acceslib.h iogen.h
	cc -o sample8 sample8.c acceslib.c
	@echo sample8 made
