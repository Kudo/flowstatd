#
#    flowd - Netflow statistics daemon
#    Copyright (C) 2011 Kudo Chien <ckchien@gmail.com>
#
#    This program is free software; you can redistribute it and/or
#    modify it under the terms of the GNU General Public License
#    as published by the Free Software Foundation; either version 2
#    of the License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
#    Optionally you can also view the license at <http://www.gnu.org/licenses/>.
#


##########################################
##      Variable Definition
CC	= gcc
CPROTO	= cproto
CFLAGS	= -O2 -Wall
#CFLAGS	= -g -Wall
LIB	= -lz
SRC	= command.c netflow.c flowd.c fttime.c multiplex.c socket.c global.c
OBJ	= $(SRC:.c=.o)

.SUFFIXES: .o
.c.o:	;	$(CC) $(CFLAGS) -c $*.c

##########################################
##	Usage
##########################################
all:	flowd

flowd:	$(OBJ)
	$(CC) $(CFLAGS) $(LIB) -o ../bin/$@ $?

flowd.p: $(SRC)
	$(CPROTO) -o $@ $(SRC)

clean:
	-rm -f ../bin/flowd $(OBJ)

clean-all:
	-rm -f ../bin/flowd $(OBJ) *~
