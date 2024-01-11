# makedeez

flags = -Wall -Wextra -Werror -lpthread
allobj = main.o house.o room.o ghost.o hunter.o evidence.o \
	logger.o utils.o

all: program

program: $(allobj)
	gcc $(warnings) -o final $(allobj)

main.o: main.c defs.h
	gcc $(warnings) -c main.c

house.o: house.c defs.h
	gcc $(warnings) -c house.c

room.o: room.c defs.h utils.c
	gcc $(warnings) -c room.c

ghost.o: ghost.c defs.h utils.c
	gcc $(warnings) -c ghost.c

hunter.o: hunter.c defs.h utils.c
	gcc $(warnings) -c hunter.c

evidence.o: evidence.c defs.h
	gcc $(warnings) -c evidence.c

logger.o: logger.c defs.h
	gcc $(warnings) -c logger.c

utils.o: utils.c defs.h
	gcc $(warnings) -c utils.c

clean:
	rm -f final *.o *~