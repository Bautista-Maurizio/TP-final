arkanoid: Backend.o Frontend1.o Stage.o main.o
	gcc Backend.o Frontend1.o Stage.o main.o -o arkanoid \
	    -lallegro_primitives -lallegro_image -lallegro_ttf -lallegro_font -lallegro -lallegro_audio -lallegro_acodec -lm

Backend.o: Backend.c Backend.h Frontend1.h Stage.h
	gcc Backend.c -c -Wall 

Frontend1.o: Frontend1.c Frontend1.h Backend.h Stage.h
	gcc Frontend1.c -c -Wall 

Stage.o: Stage.c Stage.h Backend.h Frontend1.h
	gcc Stage.c -c -Wall 

main.o: main.c Backend.h Frontend1.h Stage.h
	gcc main.c -c -Wall 

clean:
	rm -f *.o arkanoid