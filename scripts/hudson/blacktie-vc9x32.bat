call:comment_on_pull "Started testing pull request: %BUILD_URL%"

set NOPAUSE=true

rem SHUTDOWN ANY PREVIOUS BUILD REMNANTS
if exist jboss-as\bin\jboss-cli.bat call jboss-as\bin\jboss-cli.bat --connect command=:shutdown && cd .
if exist jboss-as\bin\jboss-cli.bat @ping 127.0.0.1 -n 10 -w 1000 > nul
tasklist
taskkill /F /IM mspdbsrv.exe
taskkill /F /IM testsuite.exe
taskkill /F /IM server.exe
taskkill /F /IM client.exe
taskkill /F /IM cs.exe
tasklist

rem INITIALIZE JBOSS
cd %WORKSPACE%
call ant -f scripts/hudson/initializeJBoss.xml -DJBOSS_HOME=%JBOSS_HOME% -Dbasedir=. initializeDatabase initializeJBoss -debug
IF %ERRORLEVEL% NEQ 0 call:comment_on_pull "Build failed %BUILD_URL%" & exit -1
set JBOSS_HOME=%WORKSPACE%\jboss-as\

rem START JBOSS
start /B %JBOSS_HOME%\bin\standalone.bat -c standalone-full.xml -Djboss.bind.address=%JBOSSAS_IP_ADDR% -Djboss.bind.address.unsecure=%JBOSSAS_IP_ADDR%
echo "Started server"
@ping 127.0.0.1 -n 20 -w 1000 > nul

rem BUILD BLACKTIE
cd %WORKSPACE%
call build.bat clean install "-Djbossas.ip.addr=%JBOSSAS_IP_ADDR%"
IF %ERRORLEVEL% NEQ 0 call:comment_on_pull "Build failed %BUILD_URL%" & echo "Failing build 2" & tasklist & call %JBOSS_HOME%\bin\jboss-cli.bat --connect command=:shutdown & @ping 127.0.0.1 -n 10 -w 1000 > nul & exit -1

rem SHUTDOWN ANY PREVIOUS BUILD REMNANTS
tasklist & call %JBOSS_HOME%\bin\jboss-cli.bat --connect command=:shutdown & @ping 127.0.0.1 -n 10 -w 1000 > nul
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
