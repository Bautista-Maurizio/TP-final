arkopi: Backend.o Frontend2.o Words.o main2.o joydrv.o disdrv.o Stage.o
	gcc Backend.o Frontend2.o Words.o main2.o Stage.o joydrv.o disdrv.o -lm -o arkopi

Backend.o: Backend.c Backend.h Words.h Frontend2.h
	gcc -Wall -Ijoydisp -DPI_16x16 -c Backend.c

Frontend2.o: Frontend2.c Frontend2.h Words.h Backend.h joydrv.h disdrv.h
	gcc -Wall -Ijoydisp -DPI_16x16 -c Frontend2.c

Words.o: Words.c Words.h
	gcc -Wall -Ijoydisp -DPI_16x16 -c Words.c

main2.o: main2.c Frontend2.h Backend.h Words.h joydrv.h disdrv.h
	gcc -Wall -Ijoydisp -DPI_16x16 -c main2.c

Stage.o: Stage.c Stage.h
	gcc -Wall -DPI_16x16 -c Stage.c 

clean:
	rm -f *.o arkopi
