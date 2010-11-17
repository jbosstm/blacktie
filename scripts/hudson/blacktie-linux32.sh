# CHECK IF WORKSPACE IS SET
if [ -n "${WORKSPACE+x}" ]; then
  echo WORKSPACE is set
else
  echo WORKSPACE not set
  exit
fi

# SHUTDOWN ANY RUNNING JBOSS
if [ -d $WORKSPACE/jboss-5.1.0.GA ]; then
  echo foo | $WORKSPACE/jboss-5.1.0.GA/bin/shutdown.sh -S && cd .
  sleep 30
  rm -rf $WORKSPACE/jboss-5.1.0.GA
fi

# GET THE TNS NAMES
TNS_ADMIN=$WORKSPACE/instantclient_11_2/network/admin
mkdir -p $TNS_ADMIN
(cd $TNS_ADMIN; wget http://albany/userContent/blacktie/tnsnames.ora)

# GET JBOSS AND INITIALIZE IT
cd $WORKSPACE
wget http://albany/userContent/blacktie/jboss-5.1.0.GA.zip
unzip jboss-5.1.0.GA.zip
wget http://albany/userContent/blacktie/hornetq-2.1.2.Final.zip
echo 'A
' | unzip hornetq-2.1.2.Final.zip
# INSTALL HORNETQ
cd $WORKSPACE/hornetq-2.1.2.Final/config/jboss-as-5/
chmod 775 build.sh
export JBOSS_HOME=$WORKSPACE/jboss-5.1.0.GA && ./build.sh && export JBOSS_HOME=
# INSTALL TRANSACTIONS
cd $WORKSPACE/jboss-5.1.0.GA/docs/examples/transactions
ant jts -Dtarget.server.dir=../../../server/all-with-hornetq
if [ "$?" != "0" ]; then
	exit -1
fi
cd $WORKSPACE/jboss-5.1.0.GA/server/all-with-hornetq/conf
sed -i 's/CONFIGURATION_FILE/NAME_SERVICE/g' jbossts-properties.xml
# SET MAXIMUM
#cd $WORKSPACE/jboss-5.1.0.GA/bin
#sed -i 's=Xmx128=Xmx768=g' run.conf
#sed -i 's=Xmx512=Xmx768=g' run.conf


# INITIALIZE THE BLACKTIE JBOSS DEPENDENCIES
cp $WORKSPACE/trunk/jatmibroker-xatmi/src/test/resources/hornetq-jms.xml $WORKSPACE/jboss-5.1.0.GA/server/all-with-hornetq/conf

# CONFIGURE SECURITY FOR THE ADMIN SERVICES
sed -i 's?</security-settings>?      <security-setting match="jms.queue.BTR_BTDomainAdmin">\
         <permission type="send" roles="blacktie,guest"/>\
         <permission type="consume" roles="blacktie,guest"/>\
      </security-setting>\
      <security-setting match="jms.queue.BTR_BTStompAdmin">\
         <permission type="send" roles="blacktie,guest"/>\
         <permission type="consume" roles="blacktie,guest"/>\
      </security-setting>\
</security-settings>?g' $WORKSPACE/jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq.sar/hornetq-configuration.xml


# START JBOSS
$WORKSPACE/jboss-5.1.0.GA/bin/run.sh -c all-with-hornetq -b localhost&
sleep 53

# BUILD BLACKTIE CPP PLUGIN
cd $WORKSPACE/trunk/blacktie-utils/cpp-plugin
mvn clean install
if [ "$?" != "0" ]; then
	exit -1
fi

# BUILD BLACKTIE
cd $WORKSPACE/trunk/blacktie
# THESE ARE SEPARATE SO WE DO NOT COPY THE OLD ARTIFACTS IF THE BUILD FAILS
mvn clean
if [ "$?" != "0" ]; then
	exit -1
fi
mvn install -Dbpa=centos55x32 -Duse.valgrind=true
if [ "$?" != "0" ]; then
	exit -1
fi
# THIS IS TO RUN THE TESTS IN CODECOVERAGE
cd $WORKSPACE/trunk/jatmibroker-xatmi
mvn site
if [ "$?" != "0" ]; then
	exit -1
fi

# INITIALIZE THE BLACKTIE DISTRIBUTION
cd $WORKSPACE/trunk/scripts/test
ant dist -DBT_HOME=$WORKSPACE/trunk/dist/ -DVERSION=blacktie-3.0.0.M1-SNAPSHOT -DMACHINE_ADDR=`hostname` -DJBOSSAS_IP_ADDR=localhost -Dbpa=centos55x32
if [ "$?" != "0" ]; then
	exit -1
fi

# RUN ALL THE SAMPLES
cd $WORKSPACE/trunk/dist/blacktie-3.0.0.M1-SNAPSHOT/
. setenv.sh
if [ "$?" != "0" ]; then
	exit -1
fi
export ORACLE_HOME=/usr/lib/oracle/11.2/client
export ORACLE_LIB_DIR=/usr/lib/oracle/11.2/client/lib
export ORACLE_INC_DIR=/usr/include/oracle/11.2/client
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ORACLE_LIB_DIR
export TNS_ADMIN=$WORKSPACE/instantclient_11_2/network/admin

cp $WORKSPACE/trunk/dist/blacktie-3.0.0.M1-SNAPSHOT/examples/xatmi/security/hornetq-*.properties $WORKSPACE/jboss-5.1.0.GA/server/all-with-hornetq/conf/props
sed -i 's?</security-settings>?      <security-setting match="jms.queue.BTR_SECURE">\
         <permission type="send" roles="blacktie"/>\
         <permission type="consume" roles="blacktie"/>\
      </security-setting>\
</security-settings>?g' $WORKSPACE/jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq.sar/hornetq-configuration.xml

./run_all_samples.sh tx
if [ "$?" != "0" ]; then
	exit -1
fi

# SHUTDOWN JBOSS (THIS SHOULD ALWAYS HAPPEN IDEALLY)
echo foo | $WORKSPACE/jboss-5.1.0.GA/bin/shutdown.sh -S && cd .
