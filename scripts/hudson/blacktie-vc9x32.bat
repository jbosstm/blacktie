set NOPAUSE=true

rem SHUTDOWN ANY PREVIOUS BUILD REMNANTS
if exist jboss-as-7.1.1.Final call %WORKSPACE%\jboss-as-7.1.1.Final\bin\jboss-cli.bat --connect command=:shutdown && cd .
if exist jboss-as-7.1.1.Final @ping 127.0.0.1 -n 10 -w 1000 > nul
tasklist
taskkill /F /IM mspdbsrv.exe
taskkill /F /IM testsuite.exe
taskkill /F /IM server.exe
taskkill /F /IM client.exe
taskkill /F /IM cs.exe
tasklist

rem INITIALIZE JBOSS
call %WORKSPACE%\scripts\hudson\initializeJBoss.bat
IF %ERRORLEVEL% NEQ 0 exit -1

rem START JBOSS
cd %WORKSPACE%\jboss-as-7.1.1.Final\bin
start /B standalone.bat -c standalone-full.xml -Djboss.bind.address=%JBOSSAS_IP_ADDR% -Djboss.bind.address.unsecure=%JBOSSAS_IP_ADDR%
echo "Started server"
@ping 127.0.0.1 -n 20 -w 1000 > nul

rem BUILD BLACKTIE
cd %WORKSPACE%
call build.bat clean install "-Dbpa=vc9x32" "-Djbossas.ip.addr=%JBOSSAS_IP_ADDR%"
IF %ERRORLEVEL% NEQ 0 echo "Failing build 2" & tasklist & call %WORKSPACE%\jboss-as-7.1.1.Final\bin\jboss-cli.bat --connect command=:shutdown & @ping 127.0.0.1 -n 10 -w 1000 > nul & exit -1

rem CREATE BLACKTIE DISTRIBUTION
cd %WORKSPACE%\scripts\test
for /f "delims=" %%a in ('hostname') do @set MACHINE_ADDR=%%a
call ant dist -DBT_HOME=%WORKSPACE%\dist\ -DVERSION=blacktie-5.0.0.M2-SNAPSHOT -DJBOSSAS_IP_ADDR=%JBOSSAS_IP_ADDR% -DMACHINE_ADDR=%MACHINE_ADDR% -Dbpa=vc9x32
IF %ERRORLEVEL% NEQ 0 echo "Failing build 3" & tasklist & call %WORKSPACE%\jboss-as-7.1.1.Final\bin\jboss-cli.bat --connect command=:shutdown & @ping 127.0.0.1 -n 10 -w 1000 > nul & exit -1
rem tasklist & call %WORKSPACE%\jboss-as-7.1.1.Final\bin\shutdown.bat -s %JBOSSAS_IP_ADDR%:1099 -S & echo "Failed build" & 

rem RUN THE SAMPLES
cd %WORKSPACE%\dist\blacktie-5.0.0.M2-SNAPSHOT
IF %ERRORLEVEL% NEQ 0 echo "Failing build 4" & tasklist & call %WORKSPACE%\jboss-as-7.1.1.Final\bin\jboss-cli.bat --connect command=:shutdown & @ping 127.0.0.1 -n 10 -w 1000 > nul & exit -1

set PATH=%PATH%;%ORACLE_HOME%\bin;%ORACLE_HOME%\vc9

set PATH=%PATH%;%WORKSPACE%\tools\maven\bin

echo calling generated setenv - error %ERRORLEVEL%
dir setenv.bat
call setenv.bat
IF %ERRORLEVEL% NEQ 0 echo "Failing build 5 with error %ERRORLEVEL%" & tasklist & call %WORKSPACE%\jboss-as-7.1.1.Final\bin\jboss-cli.bat --connect command=:shutdown & @ping 127.0.0.1 -n 10 -w 1000 > nul & exit -1
call run_all_quickstarts.bat tx
IF %ERRORLEVEL% NEQ 0 echo "Failing build 6 with error %ERRORLEVEL%" & tasklist & call %WORKSPACE%\jboss-as-7.1.1.Final\bin\jboss-cli.bat --connect command=:shutdown & @ping 127.0.0.1 -n 10 -w 1000 > nul & exit -1

rem SHUTDOWN ANY PREVIOUS BUILD REMNANTS
tasklist & call %WORKSPACE%\jboss-as-7.1.1.Final\bin\jboss-cli.bat --connect command=:shutdown & @ping 127.0.0.1 -n 10 -w 1000 > nul
echo "Finished build"
