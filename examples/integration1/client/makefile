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

client: client.o

clean:
	rm -f client client.o

all: log4cxx.properties client