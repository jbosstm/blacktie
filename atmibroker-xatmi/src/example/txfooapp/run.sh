# ALLOW JOBS TO BE BACKGROUNDED
set -m

if [ "$1" ]; then
if [ "$1" = "tx" ]; then
echo "Example: Running txfooapp"
shift

# RUN THE FOOAPP SERVER
cd $BLACKTIE_HOME/examples/xatmi/txfooapp
generate_server -Dservice.names=BAR -Dserver.includes="request.c ora.c DbService.c" -Dx.inc.dir="$ORACLE_INC_DIR" -Dx.lib.dir="$ORACLE_LIB_DIR" -Dx.libs="occi clntsh" -Dx.define="ORACLE"
if [ "$?" != "0" ]; then
        exit -1
fi
export BLACKTIE_CONFIGURATION=linux
# use a different logfile for the server
export LOG4CXXCONFIG=log4cxx.server.properties
btadmin startup
if [ "$?" != "0" ]; then
        exit -1
fi

#the client for this test needs to act as a server
#unset BLACKTIE_CONFIGURATION

# RUN THE C CLIENT
cd $BLACKTIE_HOME/examples/xatmi/txfooapp
generate_client -Dclient.includes="client.c request.c ora.c cutil.c" -Dx.inc.dir="$ORACLE_INC_DIR" -Dx.lib.dir="$ORACLE_LIB_DIR" -Dx.libs="occi clntsh" -Dx.define="ORACLE"
# use the default logfile for the client
unset LOG4CXXCONFIG
./client

# SHUTDOWN THE SERVER RUNNING THE btadmin TOOL
export BLACKTIE_CONFIGURATION=linux
btadmin shutdown
if [ "$?" != "0" ]; then
	exit -1
fi
unset BLACKTIE_CONFIGURATION

fi
fi
