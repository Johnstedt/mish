CC=gcc
CFLAGS=-Wall
FILES=mish.c parser.c execute.c 

mish: $(FILES)
	$(CC) $(CFLAGS) -o mish $(FILES)

clean:	
	rm *.o 
