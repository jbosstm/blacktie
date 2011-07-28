# ALLOW JOBS TO BE BACKGROUNDED
set -m

echo "Example: Running CBR"

cd $BLACKTIE_HOME/examples/cbr

# BUILD THE CBR TestOne SERVER
generate_server -Dservice.names=CBR_TestOne -Dserver.includes=TestOneService.c -Dserver.name=server_one
if [ "$?" != "0" ]; then
	exit -1
fi
mv server server_one

# BUILD THE CBR TestTwo SERVER
generate_server -Dservice.names=CBR_TestTwo -Dserver.includes=TestTwoService.c -Dserver.name=server_two
if [ "$?" != "0" ]; then
	exit -1
fi
mv server server_two

# BUILD CLIENT
generate_client -Dclient.includes=client.c

# BUILD ESB AND DEPLOY
mvn install
if [ "$?" != "0" ]; then
    exit -1
fi

# WAIT
sleep 10

# RUN TestOne AND TestTwo SERVER
btadmin startup
if [ "$?" != "0" ]; then
	mvn jboss:undeploy
    exit -1
fi

# RUN THE C CLIENT
result=`./client`
if [ "$?" != "0" ]; then
	killall -9 server_one
	killall -9 server_two
	mvn jboss:undeploy
	exit -1
fi

# SHUTDOWN SERVERS
btadmin shutdown
if [ "$?" != "0" ]; then
	mvn jboss:undeploy
	exit -1
fi

# UNDEPLOY ESB
mvn jboss:undeploy
if [ "$?" != "0" ]; then
	exit -1
fi

# CHECK RESULT
if [ "$result" != "TestOneTestTwo" ]; then
	echo "result is $result, not expect TestOneTestTwo"
	exit -1
fi
