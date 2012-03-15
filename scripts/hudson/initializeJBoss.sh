if [ -z "${WORKSPACE}" ]; then
  echo "UNSET WORKSPACE"
  exit -1;
fi

#if [ -e $WORKSPACE/jbossesb-4.9.zip ]; then
#	echo "JBossESB already downloaded"
#else
#	wget http://albany/userContent/blacktie/jbossesb-4.9.zip
#fi
#echo 'A
#' | unzip jbossesb-4.9.zip
#

ant -f scripts/hudson/initializeJBoss.xml -Dbasedir=. initializeJBoss -debug

chmod u+x $WORKSPACE/jboss-as-7.1.1.Final/bin/standalone.sh
chmod u+x $WORKSPACE/jboss-as-7.1.1.Final/bin/add-user.sh

(cd $WORKSPACE/jboss-as-7.1.1.Final/bin/ && JBOSS_HOME= ./add-user.sh admin password --silent=true)
(cd $WORKSPACE/jboss-as-7.1.1.Final/bin/ && JBOSS_HOME= ./add-user.sh guest password -a --silent=true)
(cd $WORKSPACE/jboss-as-7.1.1.Final/bin/ && JBOSS_HOME= ./add-user.sh dynsub password -a --silent=true)
