# ALLOW JOBS TO BE BACKGROUNDED
set -m

echo "Example: Running fooapp"
# RUN THE FOOAPP SERVER
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

# SHUTDOWN THE SERVER RUNNING THE XATMI ADMIN CLIENT
cd $BLACKTIE_HOME/examples/admin/xatmi
generate_client -Dclient.includes=client.c
echo '0
0
0
0
1' | ./client
# SHUTDOWN THE SERVER RUNNING THE XATMI ADMIN CLIENT
cd $BLACKTIE_HOME/examples/admin/xatmi
generate_client -Dclient.includes=client.c
echo '0
0
0
0
2' | ./client
# PICK UP THE CLOSING SERVER
sleep 3
