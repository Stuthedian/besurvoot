CC=gcc
CFLAGS = -g -std=c11 -Wall -Wfatal-errors -Wsign-compare -fstack-protector-all

bindir = bin
objdir = bin/obj
sources = check.c linked_list.c menu.c main.c
objects = $(addprefix $(objdir)/, $(sources:.c=.o))
target = $(bindir)/besurvoot

.PHONY: all
all: $(target) 
$(objdir)/check.o: check.c check.h
	$(CC) $(CFLAGS) $< -c -o $@
$(objdir)/linked_list.o: linked_list.c linked_list.h check.h
	$(CC) $(CFLAGS) $< -c -o $@
$(objdir)/menu.o: menu.c menu.h check.h linked_list.h 
	$(CC) $(CFLAGS) $< -c -o $@
$(objdir)/main.o: main.c menu.h
	$(CC) $(CFLAGS) $< -c -o $@
$(target): $(objects)
	$(CC) $(CFLAGS) $^ -o $@ -lncurses


.PHONY: clean
clean:
	-rm $(target) $(objects) 2>/dev/null
	
.PHONY: rebuild
rebuild:
	make clean && make	
