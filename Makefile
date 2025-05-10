#!/bin/make -f

.PHONY: all demos clean
include guess.mk

CFLAGS += $(cflags) -I..
objects = $(sources:.c=.o)

all: $(library) demos

$(library): $(objects)

	$(AR) r $(library) $(objects)

-include $(objects:.o=.d)
%.o: %.c

	$(CC) $(CFLAGS) -c $<

demos:

	cd demos; $(MAKE)

clean:

	$(RM) *.o *.d $(library)
	cd demos; $(MAKE) clean
