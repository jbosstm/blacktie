@echo off

echo "Example: Running svrStartFunction"

cd %BLACKTIE_HOME%\examples\xatmi\svrStartfunction

rem RUN THE FOOAPP SERVER
call generate_server -Dservice.names=BAR -Dserver.includes=BarService.c,SvrInit.c
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=win32
call btadmin startup
IF %ERRORLEVEL% NEQ 0 exit -1
set BLACKTIE_CONFIGURATION=

call btadmin shutdown
IF %ERRORLEVEL% NEQ 0 exit -1