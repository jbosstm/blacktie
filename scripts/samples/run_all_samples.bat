@echo off

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

rem SHUTDOWN USING btadmin
cd %BLACKTIE_HOME%\examples\xatmi\fooapp
call btadmin shutdown
IF %ERRORLEVEL% NEQ 0 exit -1
rem RUN THE ADMIN JMX CLIENT
rem cd %BLACKTIE_HOME%\examples\admin\jmx
rem echo '0
rem 0
rem 0
rem 0
rem 1' | mvn test
rem IF %ERRORLEVEL% NEQ 0 exit -1
rem RUN THE ADMIN JMX CLIENT
rem cd %BLACKTIE_HOME%\examples\admin\jmx
rem echo '0
rem 0
rem 0
rem 0
rem 2' | mvn test
rem IF %ERRORLEVEL% NEQ 0 exit -1
rem PICK UP THE CLOSING SERVER
@ping 127.0.0.1 -n 3 -w 1000 > nul

rem RUN THE FOOAPP SERVER AGAIN
cd %BLACKTIE_HOME%\examples\xatmi\fooapp
set BLACKTIE_CONFIGURATION=win32
call btadmin startup
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=

rem SHUTDOWN USING btadmin
cd %BLACKTIE_HOME%\examples\xatmi\fooapp
call btadmin shutdown
IF %ERRORLEVEL% NEQ 0 exit -1
rem SHUTDOWN THE SERVER RUNNING THE XATMI ADMIN CLIENT
rem cd %BLACKTIE_HOME%\examples\admin\xatmi
rem call generate_client -Dclient.includes=client.c
rem echo '0
rem 0
rem 0
rem 0
rem 1' | client
rem SHUTDOWN THE SERVER RUNNING THE XATMI ADMIN CLIENT
rem cd %BLACKTIE_HOME%\examples\admin\xatmi
rem call generate_client -Dclient.includes=client.c
rem echo '0
rem 0
rem 0
rem 0
rem 2' | client
rem PICK UP THE CLOSING SERVER
@ping 127.0.0.1 -n 3 -w 1000 > nul

rem RUN THE SECURE SERVER
cd %BLACKTIE_HOME%\examples\security
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
IF %ERRORLEVEL% NEQ 0 exit -1
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

rem RUN THE MDB EXAMPLE
cd %BLACKTIE_HOME%\examples\mdb
mvn install
IF %ERRORLEVEL% NEQ 0 exit -1

rem LET THE USER KNOW THE OUTPUT
echo "All samples ran OK"
