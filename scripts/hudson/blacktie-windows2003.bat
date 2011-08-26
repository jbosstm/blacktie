set NOPAUSE=true

rem SHUTDOWN ANY PREVIOUS BUILD REMNANTS
if exist jboss-5.1.0.GA call jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S && cd .
if exist jboss-5.1.0.GA @ping 127.0.0.1 -n 60 -w 1000 > nul
tasklist
taskkill /F /IM mspdbsrv.exe
taskkill /F /IM testsuite.exe
taskkill /F /IM server.exe
taskkill /F /IM client.exe
taskkill /F /IM cs.exe
tasklist

rem INITIALIZE JBOSS
call %WORKSPACE%\trunk\scripts\hudson\initializeJBoss.bat
IF %ERRORLEVEL% NEQ 0 exit -1

rem START JBOSS
cd %WORKSPACE%\jboss-5.1.0.GA\bin
start /B run.bat -c all-with-hornetq -b %JBOSSAS_IP_ADDR%
echo "Started server"
@ping 127.0.0.1 -n 120 -w 1000 > nul

rem BUILD BLACKTIE
cd %WORKSPACE%\trunk
call build.bat clean
IF %ERRORLEVEL% NEQ 0 echo "Failing build" & tasklist & call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & echo "Failed build" & exit -1
set JBOSS_HOME=%WORKSPACE%\jboss-5.1.0.GA
call build.bat install "-Dbpa=vc9x32" "-Djbossas.ip.addr=%JBOSSAS_IP_ADDR%"
IF %ERRORLEVEL% NEQ 0 echo "Failing build" & tasklist & call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & echo "Failed build" & exit -1
set JBOSS_HOME=

rem CREATE BLACKTIE DISTRIBUTION
cd %WORKSPACE%\trunk\scripts\test
for /f "delims=" %%a in ('hostname') do @set MACHINE_ADDR=%%a
call ant dist -DBT_HOME=%BT_HOME% -DVERSION=blacktie-5.0.0.M2-SNAPSHOT -DJBOSSAS_IP_ADDR=%JBOSSAS_IP_ADDR% -DMACHINE_ADDR=%MACHINE_ADDR%
IF %ERRORLEVEL% NEQ 0 echo "Failing build" & tasklist & call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & echo "Failed build" & exit -1

rem RUN THE SAMPLES
cd %WORKSPACE%
call ant initializeBlackTieQuickstartSecurity
cd %WORKSPACE%\trunk\dist\blacktie-5.0.0.M2-SNAPSHOT
IF %ERRORLEVEL% NEQ 0 echo "Failing build" & tasklist & call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & echo "Failed build" & exit -1
set ORACLE_HOME=C:\hudson\workspace\blacktie-windows2003\instantclient_11_2
set TNS_ADMIN=C:\hudson\workspace\blacktie-windows2003\instantclient_11_2\network\admin
set PATH=%PATH%;%ORACLE_HOME%\bin;%ORACLE_HOME%\vc9

set PATH=%PATH%;%WORKSPACE%\trunk\tools\maven\bin

call setenv.bat
IF %ERRORLEVEL% NEQ 0 echo "Failing build" & tasklist & call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & echo "Failed build" & exit -1
copy /Y %WORKSPACE%\trunk\dist\blacktie-5.0.0.M2-SNAPSHOT\quickstarts\xatmi\security\hornetq-*.properties %WORKSPACE%\jboss-5.1.0.GA\server\all-with-hornetq\conf\props
IF %ERRORLEVEL% NEQ 0 echo "Failing build" & tasklist & call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & echo "Failed build" & exit -1
call run_all_quickstarts.bat tx
IF %ERRORLEVEL% NEQ 0 echo "Failing build" & tasklist & call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & echo "Failed build" & exit -1

rem SHUTDOWN ANY PREVIOUS BUILD REMNANTS
tasklist
call %WORKSPACE%\jboss-5.1.0.GA\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S && cd .
@ping 127.0.0.1 -n 60 -w 1000 > nul
taskkill /F /IM mspdbsrv.exe
taskkill /F /IM testsuite.exe
taskkill /F /IM server.exe
taskkill /F /IM client.exe
taskkill /F /IM cs.exe
tasklist
echo "Finished build"
