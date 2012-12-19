call:comment_on_pull "Started testing this pull request: %BUILD_URL%"

set NOPAUSE=true

rem SHUTDOWN ANY PREVIOUS BUILD REMNANTS
FOR /F "usebackq token=5" %%i in (`"netstat -ano|findstr 9999.*LISTENING"`) DO @set JBOSSAS_PID=%%i & @taskkill /F /PID %JBOSSAS_PID%
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
IF %ERRORLEVEL% NEQ 0 call:comment_on_pull "Build failed %BUILD_URL%" & exit -1

cd %WORKSPACE%\jboss-as-7.1.1.Final\bin\
call add-user admin password1@ --silent=true
IF %ERRORLEVEL% NEQ 0 call:comment_on_pull "Build failed %BUILD_URL%" & exit -1
call add-user guest password1@ -a --silent=true
IF %ERRORLEVEL% NEQ 0 call:comment_on_pull "Build failed %BUILD_URL%" & exit -1
call add-user dynsub password1@ -a --silent=true
IF %ERRORLEVEL% NEQ 0 call:comment_on_pull "Build failed %BUILD_URL%" & exit -1


rem START JBOSS
cd %WORKSPACE%\jboss-as-7.1.1.Final\bin
start /B standalone.bat -c standalone-full.xml -Djboss.bind.address=%JBOSSAS_IP_ADDR% -Djboss.bind.address.unsecure=%JBOSSAS_IP_ADDR%
echo "Started server"
@ping 127.0.0.1 -n 20 -w 1000 > nul

rem BUILD BLACKTIE
cd %WORKSPACE%
call build.bat clean install "-Djbossas.ip.addr=%JBOSSAS_IP_ADDR%"
IF %ERRORLEVEL% NEQ 0 call:comment_on_pull "Build failed %BUILD_URL%" & echo "Failing build 2" & tasklist & FOR /F "usebackq token=5" %%i in (`"netstat -ano|findstr 9999.*LISTENING"`) DO @set JBOSSAS_PID=%%i & @taskkill /F /PID %JBOSSAS_PID% & @ping 127.0.0.1 -n 10 -w 1000 > nul & exit -1

rem SHUTDOWN ANY PREVIOUS BUILD REMNANTS
tasklist & FOR /F "usebackq token=5" %%i in (`"netstat -ano|findstr 9999.*LISTENING"`) DO @set JBOSSAS_PID=%%i & @taskkill /F /PID %JBOSSAS_PID% & @ping 127.0.0.1 -n 10 -w 1000 > nul
echo "Finished build"

call:comment_on_pull "Tests Passed: %BUILD_URL%"


rem -------------------------------------------------------
rem -                 Functions bellow                    -
rem -------------------------------------------------------

goto:eof

:comment_on_pull
   if not "%COMMENT_ON_PULL%"=="1" goto:eof

   for /f "tokens=1,2,3,4 delims=/" %%a in ("%GIT_BRANCH%") do set IS_PULL=%%b&set PULL_NUM=%%c
   if not "%IS_PULL%"=="pull" goto:eof
   
   curl -d "{ \"body\": \"%~1\" }" -ujbosstm-bot:%BOT_PASSWORD% https://api.github.com/repos/%GIT_ACCOUNT%/%GIT_REPO%/issues/%PULL_NUM%/comments

goto:eof
