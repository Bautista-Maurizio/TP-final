# makefile.mak — Arkanoid (sin audio)
# Uso:
#   make -f makefile.mak        # compila
#   make -f makefile.mak run    # compila y ejecuta
#   make -f makefile.mak clean  # limpia

APP       := arkanoid
SRC       := Backend.c Frontend1.c main.c
OBJ       := $(SRC:.c=.o)

CFLAGS    := -std=c11 -O2 -Wall -Wextra -Wpedantic

# Allegro (sin audio) via pkg-config
ALLEGRO_PKGS := allegro-5 allegro_primitives-5 allegro_image-5 allegro_font-5 allegro_ttf-5
CFLAGS      += $(shell pkg-config --cflags $(ALLEGRO_PKGS))
LDFLAGS     := $(shell pkg-config --libs   $(ALLEGRO_PKGS)) -lm

all: $(APP)

$(APP): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(APP)
	./$(APP)

clean:
	rm -f $(OBJ) $(APP)