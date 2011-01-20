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
LIBS=-L$(LDIR) -latmibroker-xatmi -latmibroker-core -latmibroker-tx

all: queues txsender

queues: queues.c
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

txsender: txsender.c
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f queues txsender
