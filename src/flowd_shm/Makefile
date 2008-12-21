##########################################
##	Makefile
##	By Kudo@CCU
##	kudo@cna.ccu.edu.tw
##########################################
##      Variable Definition
CC	= gcc
CPROTO	= cproto
CFLAGS	= -O2 -Wall
#CFLAGS	= -g -Wall
SRC	= flowd.c fttime.c
OBJ	= $(SRC:.c=.o)

.SUFFIXES: .o
.c.o:	;	$(CC) $(CFLAGS) -c $*.c

##########################################
##	Usage
##########################################
all:	flowd

flowd:	$(SRC)
	$(CC) $(CFLAGS) -o $@ $?

flowd.p:	$(SRC)
	$(CPROTO) -o $@ $?

clean:
	-rm -f flowd $(OBJ)
