# ---------- makefile2.mak ----------
CC      := gcc
# Común a ambos targets
CFLAGS  := -std=c99 -O2 -Wall

# Archivos compartidos (incluye Stage)
SRCS_COMMON := Backend.c Stage.c Words.c

# ---------- Build Raspberry Pi (matriz 16x16) ----------
# Usa joydisp y activa layout compacto del backend
TARGET_PI   := arkopi
SRCS_PI     := $(SRCS_COMMON) Frontend2.c main2.c
OBJS_PI     := $(SRCS_PI:.c=.o) joydrv.o disdrv.o
CFLAGS_PI := $(CFLAGS) -Ijoydisp -DPI_16x16 -D_XOPEN_SOURCE=700
LDFLAGS_PI  := -lm

$(TARGET_PI): $(OBJS_PI)
	$(CC) $(CFLAGS_PI) -o $@ $(OBJS_PI) $(LDFLAGS_PI)

# ---------- Build PC (Allegro) ----------
# Para probar con Allegro en la compu
TARGET_PC   := arkopc
SRCS_PC     := $(SRCS_COMMON) Frontend2.c main2.c
# Si tenés un Frontend Allegro separado, reemplazá Frontend2.c por tu Frontend Allegro.
CFLAGS_PC   := $(CFLAGS) `pkg-config --cflags allegro-5 allegro_font-5 allegro_primitives-5`
LDFLAGS_PC  := `pkg-config --libs   allegro-5 allegro_font-5 allegro_primitives-5` -lm

$(TARGET_PC): $(SRCS_PC:.c=.o)
	$(CC) $(CFLAGS_PC) -o $@ $^ $(LDFLAGS_PC)

# ---------- Regla genérica ----------
%.o: %.c
	$(CC) $(CFLAGS_PI) -c $< -o $@

# (Si querés compilar objetos para PC, hacé: make clean && make arkopc)
# ---------- Limpieza ----------
.PHONY: clean
clean:
	rm -f *.o $(TARGET_PI) $(TARGET_PC)
# -----------------------------------