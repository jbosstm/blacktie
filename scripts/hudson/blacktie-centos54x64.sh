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
for i in `ps -eaf | grep java | grep "standalone-full.xml" | grep -v grep | cut -c10-15`; do kill -9 $i; done
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
. $WORKSPACE/scripts/hudson/initializeJBoss.sh
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
	for i in `ps -eaf | grep java | grep "standalone-full.xml" | grep -v grep | cut -c10-15`; do kill -9 $i; done
	killall -9 testsuite
	killall -9 server
	killall -9 client
	killall -9 cs
  ps -f
	exit -1
fi

# INITIALIZE THE BLACKTIE DISTRIBUTION
ant -f $WORKSPACE/scripts/test/build.xml dist -DBT_HOME=$WORKSPACE/dist/ -DVERSION=blacktie-5.0.0.M2-SNAPSHOT -DMACHINE_ADDR=`hostname` -DJBOSSAS_IP_ADDR=$JBOSSAS_IP_ADDR -Dbpa=centos54x64
if [ "$?" != "0" ]; then
	ps -f
	for i in `ps -eaf | grep java | grep "standalone-full.xml" | grep -v grep | cut -c10-15`; do kill -9 $i; done
	killall -9 testsuite
	killall -9 server
	killall -9 client
	killall -9 cs
  ps -f
	exit -1
fi

# RUN ALL THE SAMPLES
cd $WORKSPACE/dist/blacktie-5.0.0.M2-SNAPSHOT/
chmod u+x setenv.sh
. ./setenv.sh
if [ "$?" != "0" ]; then
	ps -f
	for i in `ps -eaf | grep java | grep "standalone-full.xml" | grep -v grep | cut -c10-15`; do kill -9 $i; done
	killall -9 testsuite
	killall -9 server
	killall -9 client
	killall -9 cs
  ps -f
	exit -1
fi
export ORACLE_HOME=/usr/lib/oracle/11.2/client64
export ORACLE_LIB_DIR=/usr/lib/oracle/11.2/client64/lib
export ORACLE_INC_DIR=/usr/include/oracle/11.2/client64
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ORACLE_LIB_DIR
export TNS_ADMIN=$WORKSPACE/instantclient_11_2/network/admin

export DB2DIR=/opt/ibm/db2/V9.7
export DB2_LIB=$DB2DIR/lib64
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DB2_LIB

export PATH=$PATH:$WORKSPACE/tools/maven/bin

./run_all_quickstarts.sh tx
if [ "$?" != "0" ]; then
	ps -f
	for i in `ps -eaf | grep java | grep "standalone-full.xml" | grep -v grep | cut -c10-15`; do kill -9 $i; done
	killall -9 testsuite
	killall -9 server
	killall -9 client
	killall -9 cs
  ps -f
	exit -1
fi

# KILL ANY BUILD REMNANTS
ps -f
for i in `ps -eaf | grep java | grep "standalone-full.xml" | grep -v grep | cut -c10-15`; do kill -9 $i; done
killall -9 testsuite
killall -9 server
killall -9 client
killall -9 cs
ps -f
