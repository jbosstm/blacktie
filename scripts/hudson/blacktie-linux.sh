ulimit -c unlimited

# CHECK IF WORKSPACE IS SET
if [ -n "${WORKSPACE+x}" ]; then
  echo WORKSPACE is set
else
  echo WORKSPACE not set
  exit
fi

# KILL ANY PREVIOUS BUILD REMNANTS
ps -f
for i in `ps -eaf | grep java | grep "standalone.*xml" | grep -v grep | cut -c10-15`; do kill -9 $i; done
killall -9 testsuite
killall -9 server
killall -9 client
killall -9 cs
ps -f

# GET THE TNS NAMES
TNS_ADMIN=$WORKSPACE/instantclient_11_2/network/admin
mkdir -p $TNS_ADMIN
if [ -e $TNS_ADMIN/tnsnames.ora ]; then
	echo "tnsnames.ora already downloaded"
else
	(cd $TNS_ADMIN; wget http://albany/userContent/blacktie/tnsnames.ora)
fi

# INITIALIZE JBOSS
ant -f scripts/hudson/initializeJBoss.xml -Dbasedir=. initializeJBoss -debug
if [ "$?" != "0" ]; then
	exit -1
fi

chmod u+x $WORKSPACE/jboss-as-7.1.1.Final/bin/standalone.sh
chmod u+x $WORKSPACE/jboss-as-7.1.1.Final/bin/add-user.sh

(cd $WORKSPACE/jboss-as-7.1.1.Final/bin/ && JBOSS_HOME= ./add-user.sh admin password --silent=true)
(cd $WORKSPACE/jboss-as-7.1.1.Final/bin/ && JBOSS_HOME= ./add-user.sh guest password -a --silent=true)
(cd $WORKSPACE/jboss-as-7.1.1.Final/bin/ && JBOSS_HOME= ./add-user.sh dynsub password -a --silent=true)
if [ "$?" != "0" ]; then
	exit -1
fi

# START JBOSS
$WORKSPACE/jboss-as-7.1.1.Final/bin/standalone.sh -c standalone-full.xml -Djboss.bind.address=$JBOSSAS_IP_ADDR -Djboss.bind.address.unsecure=$JBOSSAS_IP_ADDR&
sleep 5

# BUILD BLACKTIE
cd $WORKSPACE
./build.sh clean install -Djbossas.ip.addr=$JBOSSAS_IP_ADDR "$@"
if [ "$?" != "0" ]; then
	ps -f
	for i in `ps -eaf | grep java | grep "standalone.*xml" | grep -v grep | cut -c10-15`; do kill -9 $i; done
	killall -9 testsuite
	killall -9 server
	killall -9 client
	killall -9 cs
    ps -f
	exit -1
fi

# KILL ANY BUILD REMNANTS
ps -f
for i in `ps -eaf | grep java | grep "standalone.*xml" | grep -v grep | cut -c10-15`; do kill -9 $i; done
killall -9 testsuite
killall -9 server
killall -9 client
killall -9 cs
ps -f
