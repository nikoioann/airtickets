CC=gcc
CFLAGS= -pthread -lm -g 
DEPS = header.h
DEPS_C = main.c source.c
TARGETS = main

all : $(TARGETS)

main: $(DEPS_C) $(DEPS)
	$(CC)  $^ $(CFLAGS) -o $@

clean:
	rm -f $(TARGETS) *.o
