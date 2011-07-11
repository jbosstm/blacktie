# ALLOW JOBS TO BE BACKGROUNDED
set -m

echo "Example: Running CBR"

cd $BLACKTIE_HOME/examples/cbr

# RUN THE CBR TestOne SERVER
generate_server -Dservice.names=CBR_TestOne -Dserver.includes=TestOneService.c -Dserver.name=server_one
if [ "$?" != "0" ]; then
	exit -1
fi
mv server server_one

# RUN THE CBR TestTwo SERVER
generate_server -Dservice.names=CBR_TestTwo -Dserver.includes=TestTwoService.c -Dserver.name=server_two
if [ "$?" != "0" ]; then
	exit -1
fi
mv server server_two

#export BLACKTIE_CONFIGURATION=linux
#btadmin startup
#if [ "$?" != "0" ]; then
#	exit -1
#fi
#unset BLACKTIE_CONFIGURATION

# RUN THE C CLIENT
generate_client -Dclient.includes=client.c
#./client
#if [ "$?" != "0" ]; then
#	killall -9 server_one
#	killall -9 server_two
#	exit -1
#fi

btadmin shutdown
if [ "$?" != "0" ]; then
	exit -1
fi
