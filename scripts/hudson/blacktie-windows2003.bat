rem SHUTDOWN JBOSS
if exist jboss-5.1.0.GA echo foo | call jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S && cd .
if exist jboss-5.1.0.GA @ping 127.0.0.1 -n 60 -w 1000 > nul

rem INITIALIZE JBOSS
call %WORKSPACE%\trunk\scripts\hudson\initializeJBoss.bat
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
IF %ERRORLEVEL% NEQ 0 echo foo | call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & exit -1

rem BUILD BLACKTIE
cd %WORKSPACE%\trunk\blacktie
call mvn clean 
IF %ERRORLEVEL% NEQ 0 echo foo | call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & exit -1
call mvn install -Dbpa=vc9x32 -Djbossas.ip.addr=%JBOSSAS_IP_ADDR%
IF %ERRORLEVEL% NEQ 0 echo foo | call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & exit -1
rem THIS IS TO RUN THE TESTS IN CODECOVERAGE
cd %WORKSPACE%\trunk\jatmibroker-xatmi
call mvn site -Djbossas.ip.addr=%JBOSSAS_IP_ADDR%
IF %ERRORLEVEL% NEQ 0 echo foo | call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & exit -1

rem CREATE BLACKTIE DISTRIBUTION
cd %WORKSPACE%\trunk\scripts\test
for /f "delims=" %%a in ('hostname') do @set MACHINE_ADDR=%%a
call ant dist -DBT_HOME=%BT_HOME% -DVERSION=blacktie-3.0.0.M1-SNAPSHOT -DJBOSSAS_IP_ADDR=%JBOSSAS_IP_ADDR% -DMACHINE_ADDR=%MACHINE_ADDR%
IF %ERRORLEVEL% NEQ 0 echo foo | call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & exit -1

rem RUN THE SAMPLES
cd %WORKSPACE%
call ant initializeBlackTieSampleSecurity
cd %WORKSPACE%\trunk\dist\blacktie-3.0.0.M1-SNAPSHOT
IF %ERRORLEVEL% NEQ 0 echo foo | call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & exit -1
set ORACLE_HOME=C:\hudson\workspace\blacktie-windows2003\instantclient_11_2
set TNS_ADMIN=C:\hudson\workspace\blacktie-windows2003\instantclient_11_2\network\admin
set PATH=%PATH%;%ORACLE_HOME%\bin;%ORACLE_HOME%\vc9
call setenv.bat
IF %ERRORLEVEL% NEQ 0 echo foo | call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & exit -1
copy /Y %WORKSPACE%\trunk\dist\blacktie-3.0.0.M1-SNAPSHOT\examples\xatmi\security\hornetq-*.properties %WORKSPACE%\jboss-5.1.0.GA\server\all-with-hornetq\conf\props
IF %ERRORLEVEL% NEQ 0 echo foo | call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & exit -1
call run_all_samples.bat tx
IF %ERRORLEVEL% NEQ 0 echo foo | call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & exit -1

rem SHUTDOWN JBOSS
echo foo | call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S && cd .
