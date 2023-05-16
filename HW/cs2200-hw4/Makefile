SRCDIR = src

all: Word_Generator

Word_Generator: $(SRCDIR)/main.c $(SRCDIR)/arraylist.c $(SRCDIR)/arraylist_tests.c
	gcc -g $^ -o $@

clean:
	rm -f Word_Generator
