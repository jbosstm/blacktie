rem CREATE A FILE TO DOWNLOAD JBOSS AND ORACLE DRIVER
if exist build.xml del build.xml
echo ^<project name="blacktie-dependencies"^> >> build.xml
echo    ^<target name="download"^> >> build.xml
rem echo        ^<get src="http://albany/userContent/blacktie/instantclient-basiclite-win32-11.2.0.1.0.zip" dest="./instantclient-basiclite-win32-11.2.0.1.0.zip" verbose="true" usetimestamp="true"/^> >> build.xml
rem echo        ^<get src="http://albany/userContent/blacktie/instantclient-sdk-win32-11.2.0.1.0.zip" dest="./instantclient-sdk-win32-11.2.0.1.0.zip" verbose="true" usetimestamp="true"/^> >> build.xml
rem echo        ^<delete dir="instantclient_11_2"/^> >> build.xml
rem echo        ^<unzip src="./instantclient-basiclite-win32-11.2.0.1.0.zip" dest="."/^> >> build.xml
rem echo        ^<unzip src="./instantclient-sdk-win32-11.2.0.1.0.zip" dest="."/^> >> build.xml
rem echo        ^<mkdir dir="./instantclient_11_2/network/admin"/^> >> build.xml
rem echo        ^<get src="http://albany/userContent/blacktie/tnsnames.ora" dest="./instantclient_11_2/network/admin/tnsnames.ora" verbose="true" usetimestamp="true"/^> >> build.xml
rem echo        ^<move tofile="./instantclient_11_2/OCI" file="./instantclient_11_2/sdk"/^> >> build.xml
rem echo        ^<move tofile="./instantclient_11_2/OCI/lib/MSVC2" file="./instantclient_11_2/OCI/lib/msvc"/^> >> build.xml
rem echo        ^<move tofile="./instantclient_11_2/OCI/lib/MSVC" file="./instantclient_11_2/OCI/lib/MSVC2"/^> >> build.xml
rem echo        ^<mkdir dir="./instantclient_11_2/bin"/^> >> build.xml
rem echo        ^<move todir="./instantclient_11_2/bin"^> >> build.xml
rem echo            ^<fileset dir="./instantclient_11_2/"^> >> build.xml
rem echo                ^<include name="*.dll"/^> >> build.xml
rem echo            ^</fileset^> >> build.xml
rem echo        ^</move^> >> build.xml
rem echo        ^<get src="http://albany/userContent/blacktie/jboss-5.1.0.GA.zip" dest="./jboss-5.1.0.GA.zip" verbose="true" usetimestamp="true"/^> >> build.xml
rem echo        ^<get src="http://albany/userContent/blacktie/hornetq-2.1.2.Final.zip" dest="./hornetq-2.1.2.Final.zip" verbose="true" usetimestamp="true"/^> >> build.xml
echo        ^<delete dir="jboss-5.1.0.GA"/^> >> build.xml
echo        ^<unzip src="./jboss-5.1.0.GA.zip" dest="."/^> >> build.xml
echo        ^<delete dir="hornetq-2.1.2.Final"/^> >> build.xml
echo        ^<unzip src="./hornetq-2.1.2.Final.zip" dest="."/^> >> build.xml
echo    ^</target^> >> build.xml
echo    ^<target name="replaceJBoss"^> >> build.xml
echo        ^<replaceregexp byline="true" file="jboss-5.1.0.GA/server/all-with-hornetq/conf/jbossts-properties.xml" match="CONFIGURATION_FILE" replace="NAME_SERVICE"  /^> >> build.xml
echo        ^<replaceregexp byline="true" file="jboss-5.1.0.GA/server/all-with-hornetq/conf/jacorb.properties" match="localhost" replace="${JBOSSAS_IP_ADDR}"  /^> >> build.xml
echo        ^<replaceregexp byline="true" file="jboss-5.1.0.GA/server/all-with-hornetq/conf/jboss-log4j.xml" match="&lt;root&gt;" replace="&lt;category name=&quot;org.jboss.blacktie&quot;&gt;&lt;priority value=&quot;ALL&quot; /&gt;&lt;/category&gt;&lt;root&gt;"  /^> >> build.xml
echo    ^</target^> >> build.xml

echo	^<target name="configureHornetQ"^> >> build.xml
echo        ^<replaceregexp byline="true" file="jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq.sar/hornetq-configuration.xml" match="&lt;/security-settings&gt;" replace="&lt;security-setting match=&quot;jms.queue.BTR_BTDomainAdmin&quot;&gt;         &lt;permission type=&quot;send&quot; roles=&quot;blacktie,guest&quot;/&gt;         &lt;permission type=&quot;consume&quot; roles=&quot;blacktie,guest&quot;/&gt;      &lt;/security-setting&gt;      &lt;security-setting match=&quot;jms.queue.BTR_BTStompAdmin&quot;&gt;         &lt;permission type=&quot;send&quot; roles=&quot;blacktie,guest&quot;/&gt;         &lt;permission type=&quot;consume&quot; roles=&quot;blacktie,guest&quot;/&gt;      &lt;/security-setting&gt;&lt;/security-settings&gt;"  /^> >> build.xml
echo        ^<replaceregexp byline="true" file="jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq.sar/hornetq-jms.xml" match="&lt;connection-factory name=&quot;InVMConnectionFactory&quot;&gt;" replace="&lt;connection-factory name=&quot;InVMConnectionFactory&quot;&gt;         &lt;consumer-window-size&gt;0&lt;/consumer-window-size&gt;"  /^> >> build.xml
echo        ^<replaceregexp byline="true" file="jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq.sar/hornetq-jms.xml" match="&lt;connection-factory name=&quot;InVMConnectionFactory&quot;&gt;" replace="&lt;connection-factory name=&quot;InVMConnectionFactory&quot;&gt;&#10;      &lt;connection-ttl&gt;-1&lt;/connection-ttl&gt;&#10;      &lt;client-failure-check-period&gt;86400000&lt;/client-failure-check-period&gt;" /^> >> build.xml
echo        ^<replaceregexp byline="true" file="jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq-ra.rar/META-INF/ra.xml" match="&lt;resourceadapter-class&gt;org.hornetq.ra.HornetQResourceAdapter&lt;/resourceadapter-class&gt;" replace="&lt;resourceadapter-class&gt;org.hornetq.ra.HornetQResourceAdapter&lt;/resourceadapter-class&gt;&#10;      &lt;config-property&gt;&#10;        &lt;description&gt;The connection TTL&lt;/description&gt;&#10;        &lt;config-property-name&gt;ConnectionTTL&lt;/config-property-name&gt;&#10;        &lt;config-property-type&gt;java.lang.Long&lt;/config-property-type&gt;&#10;        &lt;config-property-value&gt;-1&lt;/config-property-value&gt;&#10;      &lt;/config-property&gt;" /^> >> build.xml
echo	^</target^> >> build.xml

echo	^<target name="initializeBlackTieSampleSecurity"^> >> build.xml
echo        ^<replaceregexp byline="true" file="jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq.sar/hornetq-configuration.xml" match="&lt;/security-settings&gt;" replace="      &lt;security-setting match=&quot;jms.queue.BTR_SECURE&quot;&gt;         &lt;permission type=&quot;send&quot; roles=&quot;blacktie&quot;/&gt;         &lt;permission type=&quot;consume&quot; roles=&quot;blacktie&quot;/&gt;      &lt;/security-setting&gt;&lt;/security-settings&gt;"  /^> >> build.xml
echo	^</target^> >> build.xml
echo ^</project^> >> build.xml

rem DOWNLOAD THOSE DEPENDENCIES
cd %WORKSPACE%
call ant download
IF %ERRORLEVEL% NEQ 0 exit -1

rem INITIALIZE HORNETQ
cd %WORKSPACE%\hornetq-2.1.2.Final\config\jboss-as-5\
set JBOSS_HOME=%WORKSPACE%\jboss-5.1.0.GA
call build.bat
set JBOSS_HOME=
cd %WORKSPACE

rem INITIALIZE JBOSS
cd %WORKSPACE%\jboss-5.1.0.GA\docs\examples\transactions
call ant jts -Dtarget.server.dir=../../../server/all-with-hornetq
IF %ERRORLEVEL% NEQ 0 exit -1
cd %WORKSPACE%
call ant replaceJBoss -DJBOSSAS_IP_ADDR=%JBOSSAS_IP_ADDR%
IF %ERRORLEVEL% NEQ 0 exit -1

rem INITIALZE BLACKTIE JBOSS DEPENDENCIES
copy %WORKSPACE%\trunk\jatmibroker-xatmi\src\test\resources\hornetq-jms.xml %WORKSPACE%\jboss-5.1.0.GA\server\all-with-hornetq\conf
cd %WORKSPACE%
call ant configureHornetQ
IF %ERRORLEVEL% NEQ 0 exit -1
