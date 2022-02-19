.PHONY:	all

BENCHS =  src/priorityqueue-TSLQueue src/priorityqueue-linden src/priorityqueue-lotan

.PHONY:	clean all external $(BENCHS)
	
default: 
	$(MAKE)  lf_pq

pq-tsl_cas:
	$(MAKE) "ATOMIC=cas" src/priorityqueue-TSLQueue
pq-tsl_wcas:
	$(MAKE) "ATOMIC=wcas" src/priorityqueue-TSLQueue
pq-lotan:
	$(MAKE) src/priorityqueue-lotan
pq-linden:
	$(MAKE) src/priorityqueue-linden

lf_pq: pq-lotan pq-linden pq-tsl_cas pq-tsl_wcas

clean:
	$(MAKE) -C src/priorityqueue-TSLQueue clean
	$(MAKE) -C src/priorityqueue-lotan clean
	$(MAKE) -C src/priorityqueue-linden clean
	rm -rf build

$(BENCHS):
	$(MAKE) -C $@ $(TARGET)

