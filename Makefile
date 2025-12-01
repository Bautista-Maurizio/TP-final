# COMPILADOR

CC := gcc

# PATHS

P_BACKEND := src/backend/
P_ALLEGRO := src/allegro/
P_RASPI := src/raspi/
P_LIB := lib/

# FLAGS DE COMPILACION

CFLAGS_COMMON := -Wall -Isrc/backend/
CFLAGS_ALLEGRO := -DUSE_ALLEGRO -I$(P_ALLEGRO)
CFLAGS_RASPI := -DUSE_RASPI -DPI_16x16 -I$(P_RASPI) -I$(P_LIB) -I$(P_ALLEGRO)

# FLAGS DE LINKER

LINKER_FLAGS_ALLEGRO := -lallegro_primitives -lallegro_image -lallegro_ttf -lallegro_font -lallegro_audio -lallegro_acodec -lallegro -lm
LINKER_FLAGS_RASPI := -lm

# OBJETOS

OBJECTS_ALLEGRO := Allegro.o MainAllegro.o BackendAllegro.o StageAllegro.o
OBJECTS_RASPI := Raspi.o MainRaspi.o Words.o BackendRaspi.o StageRaspi.o $(P_LIB)disdrv.o $(P_LIB)joydrv.o

# NOMBRES DE EJECUTABLES

EXEC_ALLEGRO := arkanoid
EXEC_RASPI := arkopi

##########################################################################
# TARGETS PARA ALLEGRO													 #
##########################################################################	

allegro: $(OBJECTS_ALLEGRO)
	$(CC) $(OBJECTS_ALLEGRO) -o $(EXEC_ALLEGRO) $(LINKER_FLAGS_ALLEGRO)

BackendAllegro.o: $(P_BACKEND)Backend.c $(P_BACKEND)Backend.h $(P_ALLEGRO)Stage.h
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_ALLEGRO) -c $(P_BACKEND)Backend.c -o BackendAllegro.o

Allegro.o: $(P_ALLEGRO)Allegro.c $(P_ALLEGRO)Allegro.h $(P_BACKEND)Backend.h $(P_ALLEGRO)Stage.h
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_ALLEGRO) -c $(P_ALLEGRO)Allegro.c -o Allegro.o

StageAllegro.o: $(P_ALLEGRO)Stage.c $(P_ALLEGRO)Stage.h $(P_BACKEND)Backend.h $(P_ALLEGRO)Allegro.h
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_ALLEGRO) -c $(P_ALLEGRO)Stage.c -o StageAllegro.o

MainAllegro.o: $(P_ALLEGRO)MainAllegro.c $(P_BACKEND)Backend.h $(P_ALLEGRO)Allegro.h $(P_ALLEGRO)Stage.h
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_ALLEGRO) -c $(P_ALLEGRO)MainAllegro.c -o MainAllegro.o

##########################################################################
# TARGETS PARA RASPI													 #
##########################################################################	

raspi: $(OBJECTS_RASPI)
	$(CC) $(OBJECTS_RASPI) -o $(EXEC_RASPI) $(LINKER_FLAGS_RASPI)

BackendRaspi.o: $(P_BACKEND)Backend.c $(P_BACKEND)Backend.h $(P_RASPI)Words.h $(P_RASPI)Raspi.h $(P_LIB)joydrv.h $(P_LIB)disdrv.h
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RASPI) -c $(P_BACKEND)Backend.c -o BackendRaspi.o

Raspi.o: $(P_RASPI)Raspi.c $(P_RASPI)Raspi.h $(P_RASPI)Words.h $(P_BACKEND)Backend.h $(P_LIB)joydrv.h $(P_LIB)disdrv.h
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RASPI) -c $(P_RASPI)Raspi.c -o Raspi.o

Words.o: $(P_RASPI)Words.c $(P_RASPI)Words.h
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RASPI) -c $(P_RASPI)Words.c -o Words.o

MainRaspi.o: $(P_RASPI)MainRaspi.c $(P_RASPI)Raspi.h $(P_BACKEND)Backend.h $(P_RASPI)Words.h $(P_LIB)joydrv.h $(P_LIB)disdrv.h
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RASPI) -c $(P_RASPI)MainRaspi.c -o MainRaspi.o

StageRaspi.o: $(P_ALLEGRO)Stage.c $(P_ALLEGRO)Stage.h
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_RASPI) -c $(P_ALLEGRO)Stage.c -o StageRaspi.o 

##########################################################################

clean:
	rm -f *.o

cleanall:
	rm -f *.o $(EXEC_ALLEGRO) $(EXEC_RASPI)
