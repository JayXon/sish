CFLAGS	+= -Wall -O3 -std=c99 -D_POSIX_C_SOURCE=200809L -D_BSD_SOURCE -D_XOPEN_SOURCE=600
LDFLAGS	+=

OBJS	= main.o shell.o

.PHONY:	clean

sish: 	$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) $$LIBS -o $@

clean:
	rm -f sish $(OBJS)
