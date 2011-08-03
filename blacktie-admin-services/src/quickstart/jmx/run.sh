# ALLOW JOBS TO BE BACKGROUNDED
set -m

echo "Quickstart: Running JMX quickstart"

# RUN THE FOOAPP SERVER
cd $BLACKTIE_HOME/quickstarts/xatmi/fooapp
generate_server -Dservice.names=FOOAPP -Dserver.includes=BarService.c -Dserver.name=fooapp
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
cd $BLACKTIE_HOME/quickstarts/admin/jmx
echo '0
0
0
0
1' | mvn test
if [ "$?" != "0" ]; then
	exit -1
fi
# RUN THE ADMIN JMX CLIENT
cd $BLACKTIE_HOME/quickstarts/admin/jmx
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
