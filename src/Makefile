#
#    flowd - Netflow statistics daemon
#    Copyright (C) 2012 Kudo Chien <ckchien@gmail.com>
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
#CFLAGS	= -O2 -Wall
CFLAGS	= -g -Wall
LIB	= -lz

.SUFFIXES: .o
.c.o:	;	$(CC) $(CFLAGS) -c $*.c

##########################################
##	Source Files
##########################################
SRC	= command.c
SRC	+= netflow.c
SRC	+= netflow_handler_v5.c
SRC	+= netflow_handler_v9.c
SRC	+= flowd.c
SRC	+= fttime.c
SRC	+= multiplex.c
SRC	+= socket.c
SRC	+= global.c

OBJ	= $(SRC:.c=.o)
##########################################
##	Usage
##########################################
all:	flowd

flowd:	$(OBJ)
	$(CC) $(CFLAGS) $(LIB) -o ../bin/$@ $?

clean:
	-rm -f ../bin/flowd $(OBJ)

clean-all:
	-rm -f ../bin/flowd $(OBJ) *~
