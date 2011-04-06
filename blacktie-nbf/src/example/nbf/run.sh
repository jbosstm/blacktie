# ALLOW JOBS TO BE BACKGROUNDED
set -m

echo "Example: Running nbf example "

cd $BLACKTIE_HOME/examples/nbf

generate_server -Dservice.names=NBFEXAMPLE -Dserver.includes=NBFService.c
if [ "$?" != "0" ]; then
	exit -1
fi

export BLACKTIE_CONFIGURATION=linux
btadmin startup
if [ "$?" != "0" ]; then
	exit -1
fi
unset BLACKTIE_CONFIGURATION

# RUN THE C CLIENT
generate_client -Dclient.includes=client.c
./client
if [ "$?" != "0" ]; then
	killall -9 server
	exit -1
fi

export BLACKTIE_CONFIGURATION=linux
btadmin shutdown
if [ "$?" != "0" ]; then
	exit -1
fi
unset BLACKTIE_CONFIGURATION
