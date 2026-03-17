.PHONY: all clean

# Disable implicit rules
.SUFFIXES:

# Keep intermediate files
#.PRECIOUS: %.o

SRCDIR = src/
OBJDIR = objs/
HDRDIR = header/

SRCS=$(wildcard $(SRCDIR)*.c)
HEADERS=$(SRCS:$(SRCDIR)%.c=$(HDRDIR)%.h)
OBJS=$(SRCS:$(SRCDIR)%.c=$(OBJDIR)%.o)
PROGS = ftpclient ftpserverpool
OBJS := $(filter-out $(PROGS:%=$(OBJDIR)%.o), $(OBJS)) # to remove the objects corresponding to $(PROGS)

CC = gcc
CFLAGS = -Wall -Werror
LIBS = -lpthread
INCLDIR = -I $(HDRDIR)


ifdef NPROC
CFLAGS += -DNPROC=$(NPROC)
endif

all: gendir $(PROGS)

gendir:
	@if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi

$(OBJDIR)%.o: $(SRCDIR)%.c $(HDRDIR)ftp.h
	$(CC) $(CFLAGS) $(INCLDIR) -c -o $@ $<

$(PROGS): %: $(OBJDIR)%.o $(OBJS)
	$(CC) $^ -o $@ $(LIBS)

clean:
	rm -rf $(PROGS) $(OBJDIR)