##########################################
##	Makefile
##	By Kudo@CCU
##	kudo@cna.ccu.edu.tw
##########################################
##      Variable Definition
CC	= gcc
CPROTO	= cproto
#CFLAGS	= -O2 -Wall
CFLAGS	= -g -Wall
SRC	= command.c netflow.c flowd.c fttime.c socket.c
OBJ	= $(SRC:.c=.o)

.SUFFIXES: .o
.c.o:	;	$(CC) $(CFLAGS) -c $*.c

##########################################
##	Usage
##########################################
all:	flowd

flowd:	$(OBJ)
	$(CC) $(CFLAGS) -o ../bin/$@ $?

flowd.p: $(SRC)
	$(CPROTO) -o $@ $(SRC)

clean:
	-rm -f ../bin/flowd $(OBJ)

clean-all:
	-rm -f ../bin/flowd $(OBJ) *~
