@echo off

rem RUN THE FOOAPP SERVER
cd %BLACKTIE_HOME%\examples\xatmi\fooapp
call generate_server -Dservice.names=BAR -Dserver.includes=BarService.c
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=win32
call btadmin startup
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=

rem RUN THE JAVA CLIENT
cd %BLACKTIE_HOME%\examples\jab
echo hello | mvn test
IF %ERRORLEVEL% NEQ 0 exit -1

cd %BLACKTIE_HOME%\examples\xatmi\fooapp
call btadmin shutdown
IF %ERRORLEVEL% NEQ 0 exit -1