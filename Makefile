LIBS = -L. -L/usr/local/lib -L../libs/ipc2 -L../libs/buf -L../libs/strfifo
INCL = -I. -I../libs/ipc2

# CC = purify gcc -ggdb -g -Wall ${INCL} ${LIBS}
CC = gcc -ggdb -g -Wall ${INCL} ${LIBS}

SOURCES = tc2.c user.c event.c tcds.c log.c queue.c channel.c util.c announce.c
MODULES = tc2_accept.c tc2_read.c tc2_cmds.c
OBJECTS=$(SOURCES:.c=.o) $(MODULES:.c=.o)

tc: $(OBJECTS) libstrfifo libbuf libipc2
	${CC} -o tc2 $(OBJECTS) -lipc2 -lbuf -lstrfifo

test_user: user.o log.o test_user.c
	${CC} -o test_user test_user.c users.o log.o

libipc2:
	(cd ../libs/ipc2 ; make libipc2)

libbuf:
	(cd ../libs/buf ; make libbuf)

libstrfifo:
	(cd ../libs/strfifo ; make libstrfifo)

clean:
	-rm *.o tc2

# Dependencies
libipc2.a: libipc2
libbuf.a: libbuf
libstrfifo.a: libstrfifo
