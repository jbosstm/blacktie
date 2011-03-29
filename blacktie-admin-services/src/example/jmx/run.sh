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

# RUN THE ADMIN JMX CLIENT
echo "Example: Running Admin Tests"
cd $BLACKTIE_HOME/examples/admin/jmx
echo '0
0
0
0
1' | mvn test
if [ "$?" != "0" ]; then
	exit -1
fi
# RUN THE ADMIN JMX CLIENT
cd $BLACKTIE_HOME/examples/admin/jmx
echo '0
0
0
0
2' | mvn test
if [ "$?" != "0" ]; then
	exit -1
fi
# PICK UP THE CLOSING SERVER
sleep 3
