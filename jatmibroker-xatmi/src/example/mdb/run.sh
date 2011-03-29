# ALLOW JOBS TO BE BACKGROUNDED
set -m

# RUN THE MDB EXAMPLE
echo "Example: Running MDB examples"
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
