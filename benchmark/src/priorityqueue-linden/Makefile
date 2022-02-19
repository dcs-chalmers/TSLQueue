ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/pq-linden
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main 

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

pq-linden.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/pq-linden.o priorityqueue_linden.c

test.o: priorityqueue_linden.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: ssalloc.o  pq-linden.o test.o 
	$(CC) $(CFLAGS) $(BUILDIR)/ssalloc.o $(BUILDIR)/pq-linden.o  $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
