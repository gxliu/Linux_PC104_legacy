#Makefile for 104-DA12-8

default: all

all: sample0 sample1 sample2
	@echo made all

sample0: sample0.c acceslib.c acceslib.h iogen.h
	cc -o sample0 sample0.c acceslib.c -lm
	@echo made sample0

sample1: sample1.c acceslib.c acceslib.h iogen.h
	cc -o sample1 sample1.c acceslib.c
	@echo made sample1

sample2: sample2.c acceslib.c acceslib.h iogen.h
	cc -o sample2 sample2.c acceslib.c -lm
	@echo made sample2
