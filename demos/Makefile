#!/bin/make -f

include ../guess.mk
.PHONY: all clean

CFLAGS += -I../..
LDFLAGS += $(ldflags) -L..

sources = $(wildcard *.c)
targets = $(sources:.c=$(exe))

all: $(targets)

-include $(targets:%$(exe)=%.d)
%$(exe): %.c ../$(library)

	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:

	$(RM) $(targets) *.d
