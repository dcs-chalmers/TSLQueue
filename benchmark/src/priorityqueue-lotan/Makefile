ROOT = ../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/pq-lotan
PROF = $(ROOT)/src

.PHONY:	all clean

all:	main 

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

pq-lotan.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/pq-lotan.o priorityqueue-lotan.c

test.o: priorityqueue-lotan.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: ssalloc.o  pq-lotan.o test.o 
	$(CC) $(CFLAGS) $(BUILDIR)/ssalloc.o $(BUILDIR)/pq-lotan.o  $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
