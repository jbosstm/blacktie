rem SHUTDOWN JBOSS
if exist jboss-5.1.0.GA echo foo | call jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S && cd .
if exist jboss-5.1.0.GA @ping 127.0.0.1 -n 60 -w 1000 > nul

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
echo        ^<replaceregexp byline="true" file="jboss-5.1.0.GA/server/all-with-hornetq/conf/jboss-log4j.xml" match="&lt;root&gt;" replace="&lt;category name=&quot;org.jboss.blacktie&quot;&gt;&lt;priority value=&quot;DEBUG&quot; /&gt;&lt;/category&gt;&lt;root&gt;"  /^> >> build.xml
echo    ^</target^> >> build.xml

echo	^<target name="configureHornetQ"^> >> build.xml
echo        ^<replaceregexp byline="true" file="jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq.sar/hornetq-configuration.xml" match="&lt;/security-settings&gt;" replace="&lt;security-setting match=&quot;jms.queue.BTR_BTDomainAdmin&quot;&gt;         &lt;permission type=&quot;send&quot; roles=&quot;blacktie,guest&quot;/&gt;         &lt;permission type=&quot;consume&quot; roles=&quot;blacktie,guest&quot;/&gt;      &lt;/security-setting&gt;      &lt;security-setting match=&quot;jms.queue.BTR_BTStompAdmin&quot;&gt;         &lt;permission type=&quot;send&quot; roles=&quot;blacktie,guest&quot;/&gt;         &lt;permission type=&quot;consume&quot; roles=&quot;blacktie,guest&quot;/&gt;      &lt;/security-setting&gt;&lt;/security-settings&gt;"  /^> >> build.xml
echo        ^<replaceregexp byline="true" file="jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq.sar/hornetq-jms.xml" match="&lt;connection-factory name=&quot;NettyConnectionFactory&quot;&gt;" replace="&lt;connection-factory name=&quot;NettyConnectionFactory&quot;&gt;         &lt;consumer-window-size&gt;0&lt;/consumer-window-size&gt;"  /^> >> build.xml
echo        ^<replaceregexp byline="true" file="jboss-5.1.0.GA/server/all-with-hornetq/deploy/hornetq.sar/hornetq-jms.xml" match="&lt;connection-factory name=&quot;InVMConnectionFactory&quot;&gt;" replace="&lt;connection-factory name=&quot;InVMConnectionFactory&quot;&gt;         &lt;consumer-window-size&gt;0&lt;/consumer-window-size&gt;"  /^> >> build.xml
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

rem START JBOSS
cd %WORKSPACE%\jboss-5.1.0.GA\bin
rem set BUILD_ID=dontKillMe
start /B run.bat -c all-with-hornetq -b %JBOSSAS_IP_ADDR%
rem set BUILD_ID=
echo "Started server"
@ping 127.0.0.1 -n 120 -w 1000 > nul

rem BUILD BLACKTIE CPP PLUGIN
cd %WORKSPACE%\trunk\blacktie-utils\cpp-plugin\
call mvn install
IF %ERRORLEVEL% NEQ 0 exit -1

rem BUILD BLACKTIE
cd %WORKSPACE%\trunk\blacktie
call mvn clean 
IF %ERRORLEVEL% NEQ 0 exit -1
call mvn install -Dbpa=vc9x32 -Djbossas.ip.addr=%JBOSSAS_IP_ADDR%
IF %ERRORLEVEL% NEQ 0 exit -1
rem THIS IS TO RUN THE TESTS IN CODECOVERAGE
cd %WORKSPACE%\trunk\jatmibroker-xatmi
call mvn site -Djbossas.ip.addr=%JBOSSAS_IP_ADDR%
IF %ERRORLEVEL% NEQ 0 exit -1

rem CREATE BLACKTIE DISTRIBUTION
cd %WORKSPACE%\trunk\scripts\test
for /f "delims=" %%a in ('hostname') do @set MACHINE_ADDR=%%a
call ant dist -DBT_HOME=%BT_HOME% -DVERSION=blacktie-3.0.0.M1-SNAPSHOT -DJBOSSAS_IP_ADDR=%JBOSSAS_IP_ADDR% -DMACHINE_ADDR=%MACHINE_ADDR%
IF %ERRORLEVEL% NEQ 0 exit -1

rem RUN THE SAMPLES
cd %WORKSPACE%
call ant initializeBlackTieSampleSecurity
cd %WORKSPACE%\trunk\dist\blacktie-3.0.0.M1-SNAPSHOT
IF %ERRORLEVEL% NEQ 0 exit -1
set ORACLE_HOME=C:\hudson\workspace\blacktie-windows2003\instantclient_11_2
set TNS_ADMIN=C:\hudson\workspace\blacktie-windows2003\instantclient_11_2\network\admin
set PATH=%PATH%;%ORACLE_HOME%\bin;%ORACLE_HOME%\vc9
call setenv.bat
IF %ERRORLEVEL% NEQ 0 exit -1
copy /Y %WORKSPACE%\trunk\dist\blacktie-3.0.0.M1-SNAPSHOT\examples\xatmi\security\hornetq-*.properties %WORKSPACE%\jboss-5.1.0.GA\server\all-with-hornetq\conf\props
IF %ERRORLEVEL% NEQ 0 exit -1
call run_all_samples.bat tx
IF %ERRORLEVEL% NEQ 0 exit -1

rem SHUTDOWN JBOSS
echo foo | call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S && cd .
