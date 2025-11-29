allegro:
	make -f makefile1.mak #con make -f makefile.mak allegro compila version allegro

raspi:
	make -f makefile2.mak #con make -f makefile.mak raps compila version raspi

clean:
	make -f makefile1.mak clean
	make -f makefile2.mak clean