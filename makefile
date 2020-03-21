CC=gcc
CFLAGS = -g -Wall -Wfatal-errors -Wsign-compare -fstack-protector
.PHONY: all

bindir = bin
objdir = bin/obj
sources = menu.c main.c
objects = $(addprefix $(objdir)/, $(sources:.c=.o))
target = $(bindir)/main

all: $(target) 
$(objdir)/%.o: %.c menu.h
	$(CC) $(CFLAGS) $< -c -o $@
$(target): $(objects)
	$(CC) $(CFLAGS) $^ -o $@ -lncurses && ./$@


.PHONY: clean
clean:
	-rm $(target) $(objects) 2>/dev/null
	
.PHONY: rebuild
rebuild:
	make clean && make	
