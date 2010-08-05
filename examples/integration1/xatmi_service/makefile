CC=gcc 
CFLAGS=-Wall -I${BLACKTIE_HOME}/include
LDFLAGS=-L${BLACKTIE_HOME}/lib
LDFLAGS+=-lACE_gperf_lib
LDFLAGS+=-lACE
LDFLAGS+=-lapr-1
LDFLAGS+=-latmibroker-core
LDFLAGS+=-latmibroker-hybrid
LDFLAGS+=-latmibroker-idl-cxx
LDFLAGS+=-latmibroker-tx
LDFLAGS+=-latmibroker-xatmi
LDFLAGS+=-lexpat
LDFLAGS+=-llog4cxx
LDFLAGS+=-lTAO_AnyTypeCode
LDFLAGS+=-lTAO_CodecFactory
LDFLAGS+=-lTAO_CosNaming
LDFLAGS+=-lTAO_IDL_BE
LDFLAGS+=-lTAO_IDL_FE
LDFLAGS+=-lTAO_IORInterceptor
LDFLAGS+=-lTAO_ObjRefTemplate
LDFLAGS+=-lTAO_PI_Server
LDFLAGS+=-lTAO_PI
LDFLAGS+=-lTAO_PortableServer
LDFLAGS+=-lTAO
LDFLAGS+=-lTAO_Valuetype
LDFLAGS+=-lxerces-c

log4cxx.properties:
	jar xf ${BLACKTIE_HOME}/codeGeneration/codeGeneration.jar log4cxx.properties 

server.c:
	jar xf ${BLACKTIE_HOME}/codeGeneration/codeGeneration.jar server.c
	sed -i "s=//SERVICE_DEFINITIONS=void DEBIT (TPSVCINFO *);void CREDIT (TPSVCINFO *);=g" server.c
	sed -i "s=//SERVICE_ADVERTISEMENTS=tpadvertise(\"DEBIT\", DEBIT);tpadvertise(\"CREDIT\", CREDIT);=g" server.c
	

server: server.o DebitService.o CreditService.o

clean:
	rm -f server server.o DebitService.o CreditService.o server.c

all: log4cxx.properties server