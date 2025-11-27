arkanoid_pi: Backend.o Frontend2.o Words.o main2.o
	gcc Backend.o Frontend2.o Words.o main2.o -o arkanoid_pi \
	    joydrv.o disdrv.o -lm

Backend.o: Backend.c Backend.h Stage.h
	gcc Backend.c -c -Wall

Frontend2.o: Frontend2.c Frontend2.h Backend.h Words.h Stage.h disdrv.h joydrv.h
	gcc Frontend2.c -c -Wall

Words.o: Words.c Words.h Frontend2.h
	gcc Words.c -c -Wall

main2.o: main2.c Backend.h Frontend2.h Words.h Stage.h
	gcc main2.c -c -Wall

disdrv.o: disdrv.c disdrv.h 
	gcc -c disdrv.c -Wall

Joydrv.o: joydrv.c joydrv.h
	gcc -c joydrv.c -Wall

clean:
	rm -f *.o arkanoid_pi
