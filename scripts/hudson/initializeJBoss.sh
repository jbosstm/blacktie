if [ -z "${WORKSPACE}" ]; then
  echo "UNSET WORKSPACE"
  exit -1;
fi

# GET JBOSS AND INITIALIZE IT
if [ -d $WORKSPACE/jboss-5.1.0.GA ]; then
  rm -rf $WORKSPACE/jboss-5.1.0.GA
  rm -rf $WORKSPACE/hornetq-2.1.2.Final
  rm -rf $WORKSPACE/jbossesb-4.9
fi

cd $WORKSPACE
if [ -e $WORKSPACE/jboss-5.1.0.GA.zip ]; then
	echo "JBoss already downloaded"
else
	wget http://albany/userContent/blacktie/jboss-5.1.0.GA.zip
fi
unzip jboss-5.1.0.GA.zip
if [ -e $WORKSPACE/hornetq-2.1.2.Final.zip ]; then
	echo "HornetQ already downloaded"
else
	wget http://albany/userContent/blacktie/hornetq-2.1.2.Final.zip
fi
echo 'A
' | unzip hornetq-2.1.2.Final.zip
if [ -e $WORKSPACE/jbossesb-4.9.zip ]; then
	echo "JBossESB already downloaded"
else
	wget http://albany/userContent/blacktie/jbossesb-4.9.zip
fi
echo 'A
' | unzip jbossesb-4.9.zip

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
sed -i 's\<root>\<category name="org.jboss.narayana.blacktie"><priority value="ALL"/></category><root>\g' jboss-log4j.xml
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

# CONFIGURE HORNETQ TO NOT USE CONNECTION BUFFERING
sed -i 's?<connection-factory name="InVMConnectionFactory">?<connection-factory name="InVMConnectionFactory">\
      <consumer-window-size>0</consumer-window-size>?g' $WORKSPACE/jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq.sar/hornetq-jms.xml

#CONFIGURE HORNETQ TO NOT TIMEOUT INVM CONNECTIONS
sed -i 's?<connection-factory name="InVMConnectionFactory">?<connection-factory name="InVMConnectionFactory">\
      <connection-ttl>-1</connection-ttl>\
      <client-failure-check-period>86400000</client-failure-check-period>?g' $WORKSPACE/jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq.sar/hornetq-jms.xml
sed -i 's?<resourceadapter-class>org.hornetq.ra.HornetQResourceAdapter</resourceadapter-class>?<resourceadapter-class>org.hornetq.ra.HornetQResourceAdapter</resourceadapter-class>\
      <config-property>\
        <description>The connection TTL</description>\
        <config-property-name>ConnectionTTL</config-property-name>\
        <config-property-type>java.lang.Long</config-property-type>\
        <config-property-value>-1</config-property-value>\
      </config-property>\
      <config-property>\
        <description>The client failure check period</description>\
        <config-property-name>ClientFailureCheckPeriod</config-property-name>\
        <config-property-type>java.lang.Long</config-property-type>\
        <config-property-value>86400000</config-property-value>\
      </config-property>?g' $WORKSPACE/jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq-ra.rar/META-INF/ra.xml

#INSTALL JBossESB
cp $WORKSPACE/trunk/scripts/hudson/hornetq/jboss-as-hornetq-int.jar $WORKSPACE/jboss-5.1.0.GA/common/lib
cp $WORKSPACE/trunk/scripts/hudson/hornetq/hornetq-deployers-jboss-beans.xml $WORKSPACE/jboss-5.1.0.GA/server/all-with-hornetq/deployers
cd $WORKSPACE/jbossesb-4.9/install
cp deployment.properties-example deployment.properties
sed -i "s?/jbossesb-server-4.5.GA?$WORKSPACE/jboss-5.1.0.GA?" deployment.properties
sed -i "s?=default?=all-with-hornetq?" deployment.properties
sed -i "s?^org.jboss.esb.tomcat.home?#&?" deployment.properties
sed -i 's?/hornetq?&.sar?' build.xml
ant deploy
