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
echo        ^<delete dir="jboss-5.1.0.GA"/^> >> build.xml
echo        ^<unzip src="./jboss-5.1.0.GA.zip" dest="."/^> >> build.xml
echo    ^</target^> >> build.xml
echo    ^<target name="replaceJBoss"^> >> build.xml
echo        ^<replaceregexp byline="true" file="jboss-5.1.0.GA/server/all/conf/jbossts-properties.xml" match="CONFIGURATION_FILE" replace="NAME_SERVICE"  /^> >> build.xml
echo        ^<replaceregexp byline="true" file="jboss-5.1.0.GA/server/all/conf/jacorb.properties" match="localhost" replace="${JBOSSAS_IP_ADDR}"  /^> >> build.xml
echo    ^</target^> >> build.xml
echo	^<target name="replaceBlackTie"^> >> build.xml
echo        ^<replaceregexp byline="true" file="jboss-5.1.0.GA/server/all/conf/btconfig.xml" match="localhost" replace="${JBOSSAS_IP_ADDR}"  /^> >> build.xml
echo	^</target^> >> build.xml
echo ^</project^> >> build.xml

rem DOWNLOAD THOSE DEPENDENCIES
call ant download
IF %ERRORLEVEL% NEQ 0 exit -1

rem INITIALIZE JBOSS
cd %WORKSPACE%\jboss-5.1.0.GA\docs\examples\transactions
call ant jts
IF %ERRORLEVEL% NEQ 0 exit -1
cd %WORKSPACE%
call ant replaceJBoss -DJBOSSAS_IP_ADDR=%JBOSSAS_IP_ADDR%
IF %ERRORLEVEL% NEQ 0 exit -1

rem INITIALZE BLACKTIE JBOSS DEPENDENCIES
copy %WORKSPACE%\trunk\blacktie-admin-services\src\test\resources\btconfig.xml %WORKSPACE%\jboss-5.1.0.GA\server\all\conf
copy %WORKSPACE%\trunk\jatmibroker-xatmi\src\test\resources\jatmibroker-xatmi-test-service.xml %WORKSPACE%\jboss-5.1.0.GA\server\all\deploy
call ant replaceBlackTie -DJBOSSAS_IP_ADDR=%JBOSSAS_IP_ADDR%
IF %ERRORLEVEL% NEQ 0 exit -1

rem START JBOSS
cd %WORKSPACE%\jboss-5.1.0.GA\bin
rem set BUILD_ID=dontKillMe
start /B run.bat -c all -b %JBOSSAS_IP_ADDR%
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
call ant dist -DBT_HOME=%BT_HOME% -DVERSION=blacktie-2.0.0.CR2-SNAPSHOT -DJBOSSAS_IP_ADDR=%JBOSSAS_IP_ADDR% -DMACHINE_ADDR=%MACHINE_ADDR%
IF %ERRORLEVEL% NEQ 0 exit -1

rem RUN THE SAMPLES
cd %WORKSPACE%\trunk\dist\blacktie-2.0.0.CR2-SNAPSHOT
IF %ERRORLEVEL% NEQ 0 exit -1
set ORACLE_HOME=C:\hudson\workspace\blacktie-windows2003\instantclient_11_2
set TNS_ADMIN=C:\hudson\workspace\blacktie-windows2003\instantclient_11_2\network\admin
set PATH=%PATH%;%ORACLE_HOME%\bin;%ORACLE_HOME%\vc9
call setenv.bat
IF %ERRORLEVEL% NEQ 0 exit -1
call run_all_samples.bat tx
IF %ERRORLEVEL% NEQ 0 exit -1

rem SHUTDOWN JBOSS
echo foo | call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S && cd .