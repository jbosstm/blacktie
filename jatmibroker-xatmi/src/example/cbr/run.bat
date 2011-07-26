@echo off

echo "Running CBR example"

cd %BLACKTIE_HOME%\examples\cbr

rem BUILD THE CBR TestOne SERVER
call generate_server -Dservice.names=CBR_TestOne -Dserver.includes=TestOneService.c -Dserver.name=server_one
IF %ERRORLEVEL% NEQ 0 exit -1
rename server.exe server_one.exe

rem BUILD THE CBR TestTwo SERVER
call generate_server -Dservice.names=CBR_TestTwo -Dserver.includes=TestTwoService.c -Dserver.name=server_two
IF %ERRORLEVEL% NEQ 0 exit -1
rename server.exe server_two.exe

rem BUILD CLIENT
call generate_client -Dclient.includes=client.c
IF %ERRORLEVEL% NEQ 0 exit -1

rem  BUILD ESB AND DEPLOY
call mvn install
IF %ERRORLEVEL% NEQ 0 exit -1

rem RUN TestOne AND TestTwo SERVER
set BLACKTIE_CONFIGURATION=win32
call btadmin startup
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=

rem RUN THE CLIENT
client > result.txt

rem SHUTDOWN
call btadmin shutdown
IF %ERRORLEVEL% NEQ 0 exit -1

rem UNDEPLOY ESB
call mvn jboss:undeploy
IF %ERRORLEVEL% NEQ 0 exit -1

rem CHECK RESULT
