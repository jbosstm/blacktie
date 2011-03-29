# ALLOW JOBS TO BE BACKGROUNDED
set -m

# RUN THE FOOAPP SERVER
echo "Example: Running fooapp"
cd $BLACKTIE_HOME/examples/xatmi/fooapp
generate_server -Dservice.names=BAR -Dserver.includes=BarService.c
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
cd $BLACKTIE_HOME/examples/xatmi/fooapp
generate_client -Dclient.includes=client.c
./client
if [ "$?" != "0" ]; then
	killall -9 server
	exit -1
fi

btadmin shutdown
if [ "$?" != "0" ]; then
	exit -1
fi
