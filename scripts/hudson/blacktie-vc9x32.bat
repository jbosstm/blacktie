call:comment_on_pull "Started testing pull request: %BUILD_URL%"

set NOPAUSE=true

rem SHUTDOWN ANY PREVIOUS BUILD REMNANTS
FOR /F "usebackq tokens=5" %%i in (`"netstat -ano|findstr 9999.*LISTENING"`) DO taskkill /F /PID %%i
tasklist
taskkill /F /IM mspdbsrv.exe
taskkill /F /IM testsuite.exe
taskkill /F /IM server.exe
taskkill /F /IM client.exe
taskkill /F /IM cs.exe
tasklist

if not defined WORKSPACE echo "WORKSPACE not set" & exit -1

if not defined JBOSSAS_IP_ADDR echo "JBOSSAS_IP_ADDR not set" & JBOSSAS_IP_ADDR=localhost

rem INITIALIZE JBOSS
cd %WORKSPACE%
call ant -f scripts/hudson/initializeJBoss.xml -DJBOSS_HOME=%JBOSS_HOME% -Dbasedir=. initializeJBoss -debug
IF %ERRORLEVEL% NEQ 0 call:comment_on_pull "Build failed %BUILD_URL%" & exit -1
set JBOSS_HOME=

rem START JBOSS
cd jboss-as\bin
start /B standalone.bat -c standalone-full.xml -Djboss.bind.address=%JBOSSAS_IP_ADDR% -Djboss.bind.address.unsecure=%JBOSSAS_IP_ADDR%
echo "Started server"
@ping 127.0.0.1 -n 20 -w 1000 > nul

rem BUILD BLACKTIE
cd %WORKSPACE%
call build.bat clean install "-Djbossas.ip.addr=%JBOSSAS_IP_ADDR%"
IF %ERRORLEVEL% NEQ 0 call:comment_on_pull "Build failed %BUILD_URL%" & echo "Failing build 2" & tasklist & FOR /F "usebackq tokens=5" %%i in (`"netstat -ano|findstr 9999.*LISTENING"`) DO taskkill /F /PID %%i & exit -1

rem SHUTDOWN ANY PREVIOUS BUILD REMNANTS
tasklist & FOR /F "usebackq tokens=5" %%i in (`"netstat -ano|findstr 9999.*LISTENING"`) DO taskkill /F /PID %%i
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
