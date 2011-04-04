# ALLOW JOBS TO BE BACKGROUNDED
set -m

echo "Running MDB example"

# RUN THE MDB EXAMPLE
cd $BLACKTIE_HOME/examples/mdb
mvn package jboss:redeploy -DskipTests
if [ "$?" != "0" ]; then
	exit -1
fi
sleep 10
mvn surefire:test
if [ "$?" != "0" ]; then
	exit -1
fi
