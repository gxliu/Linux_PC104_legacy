#Makefile for 104-AIO12-8

default: all

all: sample0 sample1 sample2 sample3 sample5 sample8
	@echo All samples made

sample0: sample0.c acceslib.c acceslib.h iogen.h
	cc -o sample0 sample0.c acceslib.c
	@echo sample0 made

sample1: sample1.c acceslib.c acceslib.h iogen.h
	cc -o sample1 sample1.c acceslib.c
	@echo sample1 made

sample2: sample2.c acceslib.c acceslib.h iogen.h
	cc -o sample2 sample2.c acceslib.c -lm
	@echo sample2 made

sample3: sample3.c acceslib.c acceslib.h iogen.h
	cc -o sample3 sample3.c acceslib.c
	@echo sample3 made

sample5: sample5.c acceslib.c acceslib.h iogen.h
	cc -o sample5 sample8.c acceslib.c
	@echo sample5 made

sample8: sample8.c acceslib.c acceslib.h iogen.h
	cc -o sample8 sample8.c acceslib.c
	@echo sample8 made

