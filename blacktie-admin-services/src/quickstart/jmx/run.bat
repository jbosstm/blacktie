@echo off

echo "Quickstart: Running JMX quickstart"

rem RUN THE FOOAPP SERVER
cd %BLACKTIE_HOME%\quickstarts\xatmi\fooapp
call generate_server -Dservice.names=FOOAPP -Dserver.includes=BarService.c -Dserver.name=fooapp
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=win32
call btadmin startup
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=

rem RUN THE ADMIN JMX CLIENT
cd %BLACKTIE_HOME%\quickstarts\admin\jmx
(echo 0& echo 0& echo 0& echo 0& echo 1) | mvn test
IF %ERRORLEVEL% NEQ 0 exit -1
rem RUN THE ADMIN JMX CLIENT
cd %BLACKTIE_HOME%\quickstarts\admin\jmx
(echo 0& echo 0& echo 0& echo 0& echo 2) | mvn test
IF %ERRORLEVEL% NEQ 0 exit -1

rem PICK UP THE CLOSING SERVER
@ping 127.0.0.1 -n 3 -w 1000 > nul
