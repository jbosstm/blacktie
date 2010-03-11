@echo off

rem RUN THE FOOAPP SERVER
cd %BLACKTIE_HOME%\examples\xatmi\fooapp
call generate_server -Dservice.names=BAR -Dserver.includes=BarService.c
if ERRORLEVEL 1 exit 1
server -c win32 -i 1&
@ping 127.0.0.1 -n 3 -w 1000 > nul

rem RUN THE C CLIENT
call generate_client -Dclient.includes=client.c
client
if ERRORLEVEL 1 exit 1

rem RUN THE JAVA CLIENT
cd %BLACKTIE_HOME%\examples\jab
echo hello | mvn test
if ERRORLEVEL 1 exit -1

rem RUN THE ADMIN JMX CLIENT
cd %BLACKTIE_HOME%\examples\admin\jmx
echo '0
0
0
0
1' | mvn test
if ERRORLEVEL 1 exit -1
rem PICK UP THE CLOSING SERVER
fg

rem RUN THE FOOAPP SERVER AGAIN
cd %BLACKTIE_HOME%\examples\xatmi\fooapp
server -c win32 -i 1&
@ping 127.0.0.1 -n 3 -w 1000 > nul

rem SHUTDOWN THE SERVER RUNNING THE XATMI ADMIN CLIENT
cd %BLACKTIE_HOME%\examples\admin\xatmi
call generate_client -Dclient.includes=client.c
echo '0
0
0
0
1' | client
rem PICK UP THE CLOSING SERVER
fg

rem RUN THE SECURE SERVER
cd %BLACKTIE_HOME%\examples\security
call generate_server -Dservice.names=SECURE -Dserver.includes=BarService.c
if ERRORLEVEL 1 exit -1
export BLACKTIE_CONFIGURATION_DIR=serv
server -c win32 -i 1 secure&
@ping 127.0.0.1 -n 3 -w 1000 > nul
unset BLACKTIE_CONFIGURATION_DIR

rem RUN THE "guest" USER CLIENT
call generate_client -Dclient.includes=client.c
export BLACKTIE_CONFIGURATION_DIR=guest
client
rem This test is expected to fail so make sure the exit status was not 0
if ERRORLEVEL 1 exit -1
unset BLACKTIE_CONFIGURATION_DIR

rem RUN THE "dynsub" USER CLIENT
export BLACKTIE_CONFIGURATION_DIR=dynsub
client
if ERRORLEVEL 1 exit -1
unset BLACKTIE_CONFIGURATION_DIR

rem SHUTDOWN THE SERVER RUNNING THE XATMI ADMIN CLIENT
cd %BLACKTIE_HOME%\examples\admin\xatmi
call generate_client -Dclient.includes=client.c
unset BLACKTIE_CONFIGURATION_DIR
echo '0
0
0
0
1' | client
rem PICK UP THE CLOSING SERVER
fg

rem RUN THE MDB EXAMPLE
cd %BLACKTIE_HOME%\examples\mdb
mvn install
if ERRORLEVEL 1 exit -1

rem LET THE USER KNOW THE OUTPUT
echo "All samples ran OK"
