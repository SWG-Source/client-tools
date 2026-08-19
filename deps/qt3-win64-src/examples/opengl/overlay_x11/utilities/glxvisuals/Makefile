
all: glxvisuals
	
glxvisuals : glxvisuals.o
	cc -o glxvisuals glxvisuals.o -lGL -lXext -lX11

clean:
	-rm -f *.o
	-rm -f glxvisuals

