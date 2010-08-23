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
[[ -f $TNS_ADMIN/tnsnames.ora ]] && rm -f $TNS_ADMIN/tnsnames.ora
(cd $TNS_ADMIN; wget http://albany/userContent/blacktie/tnsnames.ora)

# GET JBOSS AND INITIALIZE IT
cd $WORKSPACE
#wget http://albany/userContent/blacktie/jboss-5.1.0.GA.zip
unzip jboss-5.1.0.GA.zip
cd $WORKSPACE/jboss-5.1.0.GA/docs/examples/transactions
ant jts
cd $WORKSPACE/jboss-5.1.0.GA/server/all/conf
cat jbossts-properties.xml | sed 's/CONFIGURATION_FILE/NAME_SERVICE/g' > jbossts-properties.xml.bak
mv jbossts-properties.xml.bak jbossts-properties.xml
cd $WORKSPACE/jboss-5.1.0.GA/bin
sed -i 's=Xmx128=Xmx768=g' run.conf
sed -i 's=Xmx512=Xmx768=g' run.conf

# INITIALIZE THE BLACKTIE JBOSS DEPENDENCIES
cp $WORKSPACE/trunk/jatmibroker-xatmi/src/test/resources/jatmibroker-xatmi-test-service.xml $WORKSPACE/jboss-5.1.0.GA/server/all/deploy
cp $WORKSPACE/trunk/blacktie-admin-services/src/test/resources/btconfig.xml $WORKSPACE/jboss-5.1.0.GA/server/all/conf

# START JBOSS
$WORKSPACE/jboss-5.1.0.GA/bin/run.sh -c all -b localhost&
sleep 53

# BUILD BLACKTIE CPP PLUGIN
cd $WORKSPACE/trunk/blacktie-utils/cpp-plugin
mvn clean install

# BUILD BLACKTIE
cd $WORKSPACE/trunk/blacktie
# THESE ARE SEPARATE SO WE DO NOT COPY THE OLD ARTIFACTS IF THE BUILD FAILS
mvn clean
mvn install -Dbpa=centos54x64
cd $WORKSPACE/trunk/jatmibroker-xatmi
mvn site -DskipTests

# INITIALIZE THE BLACKTIE DISTRIBUTION
cd $WORKSPACE/trunk/scripts/test
ant dist -DBT_HOME=$WORKSPACE/trunk/dist/ -DVERSION=blacktie-2.0.0.CR1 -DMACHINE_ADDR=`hostname` -DJBOSSAS_IP_ADDR=localhost -Dbpa=centos54x64

# RUN ALL THE SAMPLES
cd $WORKSPACE/trunk/dist/blacktie-2.0.0.CR1/
. setenv.sh
export ORACLE_HOME=/usr/lib/oracle/11.2/client64
export ORACLE_LIB_DIR=/usr/lib/oracle/11.2/client64/lib
export ORACLE_INC_DIR=/usr/include/oracle/11.2/client64
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ORACLE_LIB_DIR
export TNS_ADMIN=$WORKSPACE/instantclient_11_2/network/admin
./run_all_samples.sh tx

# SHUTDOWN JBOSS (THIS SHOULD ALWAYS HAPPEN IDEALLY)
echo foo | $WORKSPACE/jboss-5.1.0.GA/bin/shutdown.sh -S && cd .
