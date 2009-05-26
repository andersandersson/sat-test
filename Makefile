LFLAGS=-L/usr/X11R6/lib -lglfw -lm -lGL -lGLU -lX11 -lXxf86vm -lpthread -lXrandr

test : test.o
	gcc test.o -o test $(LFLAGS)

test.o : test.c
	gcc -c test.c -o test.o

clean :
	rm -f *.o
	rm -f test
	rm -f *~
