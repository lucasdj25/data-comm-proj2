CC = g++
CFLAGS = -g

router: router.o reply.o routingtable.o
	sudo $(CC) $(CFLAGS) -o $@ $^ 

%.o: %.cpp reply.h routingtable.h
	sudo $(CC) $(CFLAGS) -c $< 

clean:
	sudo rm *.o
