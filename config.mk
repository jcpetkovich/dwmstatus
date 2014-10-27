NAME = dwmstatus
VERSION = 1.2

# Customize below to fit your system

# paths
PREFIX = ${HOME}/.local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

MPDLIB = $(shell pkg-config --libs libmpdclient)
MPDFLAG = -DMPD

GLIBLIB = $(shell pkg-config --libs glib-2.0)
GLIBFLAG = $(shell pkg-config --cflags glib-2.0)


# includes and libs
INCS = -I. -I/usr/include -I${X11INC} ${GLIBFLAG}
LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 ${MPDLIB} ${GLIBLIB}

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\" ${MPDFLAG}
# CFLAGS = -ggdb -std=c99 -pedantic -Wall -O0 ${INCS} ${CPPFLAGS}
CFLAGS = -std=c99 -pedantic -Wall -Os ${INCS} ${CPPFLAGS}
# LDFLAGS = -ggdb ${LIBS}
LDFLAGS = -s ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}

# compiler and linker
CC = cc

