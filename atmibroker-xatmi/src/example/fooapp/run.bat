@echo off

echo "Example: Running fooapp"

cd %BLACKTIE_HOME%\examples\xatmi\fooapp

rem RUN THE FOOAPP SERVER
call generate_server -Dservice.names=FOOAPP -Dserver.includes=BarService.c
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=win32
call btadmin startup
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=

rem RUN THE C CLIENT
call generate_client -Dclient.includes=client.c
client
IF %ERRORLEVEL% NEQ 0 exit -1

call btadmin shutdown
IF %ERRORLEVEL% NEQ 0 exit -1
