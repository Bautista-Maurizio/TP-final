arkanoid: Backend.o Frontend1.o Stage.o main.o
	gcc Backend.o Frontend1.o Stage.o main.o -o arkanoid -lallegro_audio -lallegro_acodec -lallegro_primitives -lallegro_image -lallegro_ttf -lallegro_font -lallegro -lm

Backend.o: Backend.c Backend.h Stage.h Frontend1.h
	gcc Backend.c -c -Wall -DUSE_ALLEGRO

Frontend1.o: Frontend1.c Frontend1.h Backend.h Stage.h
	gcc Frontend1.c -c -Wall -DUSE_ALLEGRO

Stage.o: Stage.c Stage.h Backend.h Frontend1.h
	gcc Stage.c -c -Wall -DUSE_ALLEGRO

main.o: main.c Backend.h Frontend1.h Stage.h
	gcc main.c -c -Wall -DUSE_ALLEGRO

clean:
	rm -f *.o arkanoid
