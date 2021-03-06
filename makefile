CC=gcc
CFLAGS = -g -Og -std=c11 -Wall -Wfatal-errors -Wsign-compare \
				 -fstack-protector-all -Werror=implicit \
				 -Werror=format-security

bindir = bin
objdir = bin/obj
sources = linked_list.c menu.c main.c
objects = $(addprefix $(objdir)/, $(sources:.c=.o))
target = $(bindir)/besurvoot

.PHONY: all
all: directories $(target)

$(objects): baht.h

$(objdir)/linked_list.o: linked_list.c linked_list.h
	$(CC) $(CFLAGS) $< -c -o $@
$(objdir)/menu.o: menu.c menu.h linked_list.h 
	$(CC) $(CFLAGS) $< -c -o $@
$(objdir)/main.o: main.c menu.h
	$(CC) $(CFLAGS) $< -c -o $@
$(target): $(objects)
	$(CC) $(CFLAGS) $^ -o $@ -lncurses


.PHONY: directories
directories:
	@mkdir -p $(bindir) $(objdir)

.PHONY: clean
clean:
	-rm -r $(bindir) 2>/dev/null
	
.PHONY: rebuild
rebuild:
	make clean && make	
