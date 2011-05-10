# ALLOW JOBS TO BE BACKGROUNDED
set -m

if [ "$1" ]; then
if [ "$1" = "tx" ]; then
shift

echo "Example: Running txfooapp"

# RUN THE FOOAPP SERVER
cd $BLACKTIE_HOME/examples/xatmi/txfooapp

if [ -d "$DB2DIR" ]; then
    generate_server -Dservice.names=TXFOOAPP -Dserver.includes="request.c db2.c ora.c DbService.c" -Dx.inc.dir="$ORACLE_INC_DIR" -Dx.inc.dir2="$DB2DIR/include" -Dx.lib.dir="$ORACLE_LIB_DIR" -Dx.libs="occi clntsh" -Dx.lib.dir2="$DB2_LIB" -Dx.libs2="db2" -Dx.define="ORACLE,DB2" -Dserver.name=txfooap
	[[ "$?" != "0" ]] && exit 1

	generate_client -Dclient.includes="client.c request.c db2.c ora.c cutil.c" -Dx.inc.dir="$ORACLE_INC_DIR" -Dx.inc.dir2="$DB2DIR/include" -Dx.lib.dir="$ORACLE_LIB_DIR" -Dx.libs="occi clntsh" -Dx.lib.dir2="$DB2_LIB" -Dx.libs2="db2" -Dx.define="ORACLE,DB2"
	[[ "$?" != "0" ]] && exit 1
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DB2_LIB
else
	generate_server -Dservice.names=TXFOOAPP -Dserver.includes="request.c ora.c DbService.c" -Dx.inc.dir="$ORACLE_INC_DIR" -Dx.lib.dir="$ORACLE_LIB_DIR" -Dx.libs="occi clntsh" -Dx.define="ORACLE" -Dserver.name=txfooap
	[[ "$?" != "0" ]] && exit 1
	generate_client -Dclient.includes="client.c request.c ora.c cutil.c" -Dx.inc.dir="$ORACLE_INC_DIR" -Dx.lib.dir="$ORACLE_LIB_DIR" -Dx.libs="occi clntsh" -Dx.define="ORACLE"
	[[ "$?" != "0" ]] && exit 1
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
