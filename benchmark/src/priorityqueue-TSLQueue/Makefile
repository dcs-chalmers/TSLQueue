ROOT = ../..

ifeq ($(ATOMIC),wcas)
	CFLAGS += -DATOMIC_INSTRUCTION=1 
	BINS = $(BINDIR)/pq-tsl_wcas
else
	BINS = $(BINDIR)/pq-tsl_cas
endif


include $(ROOT)/common/Makefile.common

PROF = $(ROOT)/src

.PHONY:	all clean

all:	main 

ssalloc.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/ssalloc.o $(PROF)/ssalloc.c

pq-tsl.o: 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/pq-tsl.o priorityqueue-tslqueue.c

test.o: priorityqueue-tslqueue.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o $(TEST_FILE)

main: ssalloc.o  pq-tsl.o test.o 
	$(CC) $(CFLAGS) $(BUILDIR)/ssalloc.o $(BUILDIR)/pq-tsl.o  $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS)
