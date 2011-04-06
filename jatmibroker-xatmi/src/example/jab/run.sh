# ALLOW JOBS TO BE BACKGROUNDED
set -m

echo "Running JAB example"

# RUN THE FOOAPP SERVER
cd $BLACKTIE_HOME/examples/xatmi/fooapp
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

# RUN THE JAVA CLIENT
cd $BLACKTIE_HOME/examples/jab
echo hello | mvn test
if [ "$?" != "0" ]; then
	exit -1
fi

cd $BLACKTIE_HOME/examples/xatmi/fooapp
btadmin shutdown
if [ "$?" != "0" ]; then
	exit -1
fi
