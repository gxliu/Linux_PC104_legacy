#Makefile for 104-II32-4RO

default: all

all: sample1 irqsamp
	@echo made all

sample1: sample1.c acceslib.c acceslib.h iogen.h
	cc -o sample1 sample1.c acceslib.c
	@echo sample1 made

irqsamp: irqsamp.c acceslib.c acceslib.h iogen.h
	cc -o irqsamp irqsamp.c acceslib.c
	@echo irqsamp made
