CC=gcc
CFLAGS = -g -Wall -Wfatal-errors -Wsign-compare -fstack-protector
.PHONY: all

bindir = bin
objdir = bin/obj
sources = menu.c main.c check.c
objects = $(addprefix $(objdir)/, $(sources:.c=.o))
target = $(bindir)/besurvoot

all: $(target) 
$(objdir)/check.o: check.c check.h
	$(CC) $(CFLAGS) $< -c -o $@

$(objdir)/menu.o $(objdir)/main.o: menu.h
$(objdir)/menu.o: menu.c check.h
	$(CC) $(CFLAGS) $< -c -o $@
$(objdir)/main.o: main.c
	$(CC) $(CFLAGS) $< -c -o $@
$(target): $(objects)
	$(CC) $(CFLAGS) $^ -o $@ -lncurses && ./$@


.PHONY: clean
clean:
	-rm $(target) $(objects) 2>/dev/null
	
.PHONY: rebuild
rebuild:
	make clean && make	
