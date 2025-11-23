CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99
LDFLAGS =
LDLIBS  = -lallegro_main -lallegro_primitives -lallegro_image -lallegro -lm

SRC     = main.c Backend.c Frontend1.c
OBJ     = $(SRC:.c=.o)

all: arkanoid

arkanoid: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS) $(LDLIBS)

%.o: %.c Backend.h Frontend1.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) arkanoid

.PHONY: all clean