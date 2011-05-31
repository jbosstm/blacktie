# CHECK IF WORKSPACE IS SET
if [ -n "${WORKSPACE+x}" ]; then
  echo WORKSPACE is set
else
  echo WORKSPACE not set
  exit
fi

set NOPAUSE=true

# SHUTDOWN ANY RUNNING JBOSS
if [ -d $WORKSPACE/jboss-5.1.0.GA ]; then
  $WORKSPACE/jboss-5.1.0.GA/bin/shutdown.sh -S && cd .
  sleep 30
fi

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
. $WORKSPACE/trunk/scripts/hudson/initializeJBoss.sh
if [ "$?" != "0" ]; then
	exit -1
fi

# START JBOSS
export JBOSSAS_IP_ADDR=localhost
$WORKSPACE/jboss-5.1.0.GA/bin/run.sh -c all-with-hornetq -b localhost&
sleep 53

# BUILD BLACKTIE CPP PLUGIN
cd $WORKSPACE/trunk/blacktie-utils/cpp-plugin
mvn clean install
if [ "$?" != "0" ]; then
	ps -f
	$WORKSPACE/jboss-5.1.0.GA/bin/shutdown.sh -S && cd .
	killall -9 testsuite
	killall -9 server
	killall -9 client
	killall -9 cs
	exit -1
fi

# BUILD BLACKTIE
cd $WORKSPACE/trunk
# THESE ARE SEPARATE SO WE DO NOT COPY THE OLD ARTIFACTS IF THE BUILD FAILS
mvn clean
if [ "$?" != "0" ]; then
	ps -f
	$WORKSPACE/jboss-5.1.0.GA/bin/shutdown.sh -S && cd .
	killall -9 testsuite
	killall -9 server
	killall -9 client
	killall -9 cs
	exit -1
fi
export JBOSS_HOME=$WORKSPACE/jboss-5.1.0.GA
mvn install -Dbpa=centos55x32 -Duse.valgrind=true
if [ "$?" != "0" ]; then
	ps -f
	$WORKSPACE/jboss-5.1.0.GA/bin/shutdown.sh -S && cd .
	killall -9 testsuite
	killall -9 server
	killall -9 client
	killall -9 cs
	exit -1
fi
export JBOSS_HOME=
# THIS IS TO RUN THE TESTS IN CODECOVERAGE
cd $WORKSPACE/trunk/jatmibroker-xatmi
mvn site
if [ "$?" != "0" ]; then
	ps -f
	$WORKSPACE/jboss-5.1.0.GA/bin/shutdown.sh -S && cd .
	killall -9 testsuite
	killall -9 server
	killall -9 client
	killall -9 cs
	exit -1
fi

# INITIALIZE THE BLACKTIE DISTRIBUTION
cd $WORKSPACE/trunk/scripts/test
ant dist -DBT_HOME=$WORKSPACE/trunk/dist/ -DVERSION=blacktie-3.0.0.M3-SNAPSHOT -DMACHINE_ADDR=`hostname` -DJBOSSAS_IP_ADDR=localhost -Dbpa=centos55x32
if [ "$?" != "0" ]; then
	ps -f
	$WORKSPACE/jboss-5.1.0.GA/bin/shutdown.sh -S && cd .
	killall -9 testsuite
	killall -9 server
	killall -9 client
	killall -9 cs
	exit -1
fi

# RUN ALL THE SAMPLES
cd $WORKSPACE/trunk/dist/blacktie-3.0.0.M3-SNAPSHOT/
. setenv.sh
if [ "$?" != "0" ]; then
	ps -f
	$WORKSPACE/jboss-5.1.0.GA/bin/shutdown.sh -S && cd .
	killall -9 testsuite
	killall -9 server
	killall -9 client
	killall -9 cs
	exit -1
fi
export ORACLE_HOME=/usr/lib/oracle/11.2/client
export ORACLE_LIB_DIR=/usr/lib/oracle/11.2/client/lib
export ORACLE_INC_DIR=/usr/include/oracle/11.2/client
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ORACLE_LIB_DIR
export TNS_ADMIN=$WORKSPACE/instantclient_11_2/network/admin

export DB2DIR=/opt/ibm/db2/V9.7
export DB2_LIB=$DB2DIR/lib32
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DB2_LIB

cp $WORKSPACE/trunk/dist/blacktie-3.0.0.M3-SNAPSHOT/examples/xatmi/security/hornetq-*.properties $WORKSPACE/jboss-5.1.0.GA/server/all-with-hornetq/conf/props
sed -i 's?</security-settings>?      <security-setting match="jms.queue.BTR_SECURE">\
         <permission type="send" roles="blacktie"/>\
         <permission type="consume" roles="blacktie"/>\
      </security-setting>\
</security-settings>?g' $WORKSPACE/jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq.sar/hornetq-configuration.xml

./run_all_samples.sh tx
if [ "$?" != "0" ]; then
	ps -f
	$WORKSPACE/jboss-5.1.0.GA/bin/shutdown.sh -S && cd .
	killall -9 testsuite
	killall -9 server
	killall -9 client
	killall -9 cs
	exit -1
fi

ps -f
# SHUTDOWN JBOSS
$WORKSPACE/jboss-5.1.0.GA/bin/shutdown.sh -S && cd .
