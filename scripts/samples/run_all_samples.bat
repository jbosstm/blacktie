@echo off

echo "Running all samples"

rem RUN THE FOOAPP SERVER
cd %BLACKTIE_HOME%\examples\xatmi\fooapp
call generate_server -Dservice.names=BAR -Dserver.includes=BarService.c
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=win32
call btadmin startup
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=

rem RUN THE C CLIENT
call generate_client -Dclient.includes=client.c
client
IF %ERRORLEVEL% NEQ 0 exit -1

rem RUN THE JAVA CLIENT
cd %BLACKTIE_HOME%\examples\jab
echo hello | mvn test
IF %ERRORLEVEL% NEQ 0 exit -1

rem RUN THE ADMIN JMX CLIENT
cd %BLACKTIE_HOME%\examples\admin\jmx
(echo 0& echo 0& echo 0& echo 0& echo 1) | mvn test
IF %ERRORLEVEL% NEQ 0 exit -1
rem RUN THE ADMIN JMX CLIENT
cd %BLACKTIE_HOME%\examples\admin\jmx
(echo 0& echo 0& echo 0& echo 0& echo 2) | mvn test
IF %ERRORLEVEL% NEQ 0 exit -1
rem PICK UP THE CLOSING SERVER
@ping 127.0.0.1 -n 3 -w 1000 > nul

rem RUN THE FOOAPP SERVER AGAIN
cd %BLACKTIE_HOME%\examples\xatmi\fooapp
set BLACKTIE_CONFIGURATION=win32
call btadmin startup
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=

rem SHUTDOWN THE SERVER RUNNING THE XATMI ADMIN CLIENT
cd %BLACKTIE_HOME%\examples\admin\xatmi
call generate_client -Dclient.includes=client.c
(echo 0& echo 0& echo 0& echo 0& echo 1) | client
rem SHUTDOWN THE SERVER RUNNING THE XATMI ADMIN CLIENT
cd %BLACKTIE_HOME%\examples\admin\xatmi
call generate_client -Dclient.includes=client.c
(echo 0& echo 0& echo 0& echo 0& echo 2) | client
rem PICK UP THE CLOSING SERVER
@ping 127.0.0.1 -n 3 -w 1000 > nul

rem Running externally managed queue example
cd %BLACKTIE_HOME%\examples\xatmi\queues
call generate_client -Dclient.includes=queues.c -Dx.define=WIN32
client put 10
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_SERVER_ID=1
client get 5
IF %ERRORLEVEL% NEQ 0 exit -1
client get 5
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_SERVER_ID=
rem Successful

rem Running txsender queue example
cd %BLACKTIE_HOME%\examples\xatmi\queues
call generate_client -Dclient.includes=txsender.c -Dclient.executable.file=txsender -Dx.define=WIN32
IF %ERRORLEVEL% NEQ 0 exit -1
call generate_client -Dclient.includes=queues.c -Dx.define=WIN32
IF %ERRORLEVEL% NEQ 0 exit -1
(echo 1) | txsender
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_SERVER_ID=1
client get 2
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_SERVER_ID=
rem Successful

rem Running propagated transaction queue example
cd %BLACKTIE_HOME%\examples\xatmi\queues
call generate_client -Dclient.includes=queues.c -Dclient.executable.file=client
IF %ERRORLEVEL% NEQ 0 exit -1
call generate_server -Dserver.includes=BarService.c  -Dservice.names=BAR
IF %ERRORLEVEL% NEQ 0 exit -1
call generate_client -Dclient.includes=client.c -Dclient.executable.file=clientSender
call btadmin startup
IF %ERRORLEVEL% NEQ 0 exit -1
(echo 1) | clientSender
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_SERVER=myserv
set BLACKTIE_SERVER_ID=1
client get 1
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_SERVER_ID=
set BLACKTIE_SERVER=
call btadmin shutdown
IF %ERRORLEVEL% NEQ 0 exit -1
rem Successful

rem RUN THE SECURE SERVER
cd %BLACKTIE_HOME%\examples\xatmi\security
call generate_server -Dservice.names=SECURE -Dserver.includes=BarService.c
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION_DIR=serv
set BLACKTIE_CONFIGURATION=win32
call btadmin startup secure
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=
set BLACKTIE_CONFIGURATION_DIR=

rem RUN THE "guest" USER CLIENT
call generate_client -Dclient.includes=client.c
set BLACKTIE_CONFIGURATION_DIR=guest
client
rem This test is expected to fail so make sure the exit status was not 0
IF %ERRORLEVEL% EQU 0 exit -1
set BLACKTIE_CONFIGURATION_DIR=

rem RUN THE "dynsub" USER CLIENT
set BLACKTIE_CONFIGURATION_DIR=dynsub
client
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION_DIR=

rem SHUTDOWN THE SERVER RUNNING THE btadmin TOOL
set BLACKTIE_CONFIGURATION_DIR=serv
set BLACKTIE_CONFIGURATION=win32
call btadmin shutdown secure
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=
set BLACKTIE_CONFIGURATION_DIR=

rem RUN THE INTEGRATION 1 EXAMPLE
echo "Running integration 1 XATMI"
cd %BLACKTIE_HOME%\examples\integration1\xatmi_service\
call generate_server -Dservice.names=CREDIT,DEBIT -Dserver.includes="CreditService.c,DebitService.c"
IF %ERRORLEVEL% NEQ 0 exit -1
call btadmin startup
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\client\
call generate_client -Dclient.includes=client.c 
.\client 
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\xatmi_service\
call btadmin shutdown
IF %ERRORLEVEL% NEQ 0 exit -1

cd %BLACKTIE_HOME%\examples\integration1\ejb
call mvn install
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\ejb\ear
call mvn install
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\xatmi_adapter\
call mvn install
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\xatmi_adapter\ear\
call mvn install
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\client\
call generate_client -Dclient.includes=client.c 
IF %ERRORLEVEL% NEQ 0 exit -1

cd %BLACKTIE_HOME%\examples\integration1\ejb\ear
call mvn jboss:deploy
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\xatmi_adapter\ear\
call mvn jboss:deploy
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\client\
@ping 127.0.0.1 -n 10 -w 1000 > nul
.\client
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\xatmi_adapter\ear\
call mvn jboss:undeploy
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\ejb\ear
call mvn jboss:undeploy
IF %ERRORLEVEL% NEQ 0 exit -1

rem RUN THE MDB EXAMPLE
cd %BLACKTIE_HOME%\examples\mdb
call mvn package jboss:redeploy -DskipTests
IF %ERRORLEVEL% NEQ 0 exit -1
@ping 127.0.0.1 -n 5 -w 1000 > nul
call mvn surefire:test
IF %ERRORLEVEL% NEQ 0 exit -1

rem RUN THE TXFOOAPP SERVER
IF ["%1"] EQU ["tx"] (
echo "Running txfooapp"
SHIFT
cd %BLACKTIE_HOME%\examples\xatmi\txfooapp
call generate_server -Dservice.names=BAR -Dserver.includes="request.c ora.c DbService.c" -Dx.inc.dir="%ORACLE_HOME%\OCI\include" -Dx.lib.dir="%ORACLE_HOME%\OCI\lib\MSVC" -Dx.libs="oci" -Dx.define="ORACLE"
IF %ERRORLEVEL% NEQ 0 exit -1

IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=win32
call btadmin startup
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=

rem RUN THE C CLIENT
call generate_client -Dclient.includes="client.c request.c ora.c cutil.c" -Dx.inc.dir="%ORACLE_HOME%\OCI\include" -Dx.lib.dir="%ORACLE_HOME%\OCI\lib\MSVC" -Dx.libs="oci" -Dx.define="ORACLE"
client
IF %ERRORLEVEL% NEQ 0 exit -1

rem SHUTDOWN THE SERVER RUNNING THE btadmin TOOL
set BLACKTIE_CONFIGURATION=win32
call btadmin shutdown
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=
)

rem LET THE USER KNOW THE OUTPUT
cd %BLACKTIE_HOME%
echo "All samples ran OK"
