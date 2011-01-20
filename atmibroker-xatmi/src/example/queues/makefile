CC=gcc 
CFLAGS=-Wall

IDIR=$(BLACKTIE_HOME)/include
CFLAGS=-I$(IDIR)
ifneq ($(DEBUG),y)  
   CFLAGS += -g3
else
   CLFAGS += -O2
endif

LDIR=$(BLACKTIE_HOME)/lib/cxx
LDFLAGS=-L$(LDIR) -latmibroker-xatmi -latmibroker-core -latmibroker-tx

all: queues txsender

log4cxx.properties:
	jar xf $(BLACKTIE_HOME)/lib/java/codeGeneration-3.0.0.M1-SNAPSHOT.jar log4cxx.properties

queues-exe: queues.c 
	gcc -o $@ $^ $(CFLAGS) $(LDFLAGS)

queues: queues-exe log4cxx.properties

txsender: txsender.c
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f queues txsender log4cxx.properties
