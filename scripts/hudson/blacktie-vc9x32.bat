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
cd %WORKSPACE%
call ant -f scripts/hudson/initializeJBoss.xml -Dbasedir=. initializeDatabase initializeJBoss -debug
IF %ERRORLEVEL% NEQ 0 exit -1

cd %WORKSPACE%\jboss-as-7.1.1.Final\bin\
call add-user admin password --silent=true
IF %ERRORLEVEL% NEQ 0 exit -1
call add-user guest password -a --silent=true
IF %ERRORLEVEL% NEQ 0 exit -1
call add-user dynsub password -a --silent=true
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
