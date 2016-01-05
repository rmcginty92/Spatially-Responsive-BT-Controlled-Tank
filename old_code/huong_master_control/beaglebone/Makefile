# Makefile for GPIO User Space

CC = gcc
CFLAGS = -g -W -lrt

EXECUTABLES = adc_collect h_bridge master

all: adc_collect h_bridge master 


adc_collect: adc.c adc.h
	$(CC) -o $@ $^ $(CFLAGS) 

h_bridge: h_bridge.c h_bridge.h
	$(CC) -o $@ $^ $(CFLAGS) 

master: master.c master.h
	$(CC) -o $@ $^ $(CFLAGS) 

.PHONY: clean

clean:
	rm -f *.o *~ $(EXECUTABLES)
