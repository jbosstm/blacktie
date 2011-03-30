# ALLOW JOBS TO BE BACKGROUNDED
set -m

EXAMPLE_HOME=$BLACKTIE_HOME/examples/xatmi/library

echo "Example: Running library example"
# GENERATE AN EMPTY SERVER
cd $EXAMPLE_HOME
generate_server
if [ "$?" != "0" ]; then
	exit -1
fi

# GENERATE A LIBRARY WITH THE BAR SERVICE IN IT
cd $EXAMPLE_HOME
generate_library -Dlibrary.includes=BarService.c
if [ "$?" != "0" ]; then
	exit -1
fi

# RUN THE SERVER - MUST PROVIDE BLACKTIE_CONFIGURATION
# AS THIS EXAMPLE USES IT TO FIND THE CORRECT LIBRARY_NAME
cd $EXAMPLE_HOME
export BLACKTIE_CONFIGURATION=linux
btadmin startup
if [ "$?" != "0" ]; then
	exit -1
fi
unset BLACKTIE_CONFIGURATION

# RUN THE C CLIENT
cd $EXAMPLE_HOME
generate_client -Dclient.includes=client.c
./client
if [ "$?" != "0" ]; then
	killall -9 server
	exit -1
fi

# SHUTDOWN THE SERVER
btadmin shutdown
if [ "$?" != "0" ]; then
	exit -1
fi
