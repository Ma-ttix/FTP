.PHONY: all clean

# Disable implicit rules
.SUFFIXES:

# Keep intermediate files
#.PRECIOUS: %.o

CC = gcc
CFLAGS = -Wall -Werror
LDFLAGS =
LIBS += -lpthread

INCLUDE = csapp.h ftp.h
OBJS = csapp.o ftprequete.o
INCLDIR = -I.

PROGS = ftpclient ftpserverpool

ifdef NPROC
CFLAGS += -DNPROC=$(NPROC)
endif

all: $(PROGS)

%.o: %.c $(INCLUDE)
	$(CC) $(CFLAGS) $(INCLDIR) -c -o $@ $<

$(PROGS): %: %.o $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

clean:
	rm -f $(PROGS) *.o