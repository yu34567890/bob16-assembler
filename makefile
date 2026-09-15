TARGET = bob16asm
CC = gcc
CFLAGS = -Wall -Wextra -O0 -g 
SRCS = $(wildcard *.c)

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) ./shamefull_ai_slop/minipp.c $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
