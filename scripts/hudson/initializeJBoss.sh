if [ -z "${WORKSPACE}" ]; then
  echo "UNSET WORKSPACE"
  exit -1;
fi

# GET JBOSS AND INITIALIZE IT
if [ -d $WORKSPACE/jboss-5.1.0.GA ]; then
  rm -rf $WORKSPACE/jboss-5.1.0.GA
  rm -rf $WORKSPACE/hornetq-2.1.2.Final
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
sed -i 's\<root>\<category name="org.jboss.blacktie"><priority value="ALL"/></category><root>\g' jboss-log4j.xml
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
sed -i 's?<connection-factory name="NettyConnectionFactory">?<connection-factory name="NettyConnectionFactory">\
      <consumer-window-size>0</consumer-window-size>?g' $WORKSPACE/jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq.sar/hornetq-jms.xml
sed -i 's?<connection-factory name="InVMConnectionFactory">?<connection-factory name="InVMConnectionFactory">\
      <consumer-window-size>0</consumer-window-size>?g' $WORKSPACE/jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq.sar/hornetq-jms.xml

#CONFIGURE HORNETQ TO NOT TIMEOUT INVM CONNECTIONS
sed -i 's?<connection-factory name="InVMConnectionFactory">?<connection-factory name="InVMConnectionFactory">\
      <connection-ttl>-1</connection-ttl>\
      <client-failure-check-period>-1</client-failure-check-period>?g' $WORKSPACE/jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq.sar/hornetq-jms.xml


