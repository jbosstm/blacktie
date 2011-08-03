@echo off

echo "Running MDB quickstart"

rem RUN THE MDB EXAMPLE
cd %BLACKTIE_HOME%\quickstarts\mdb
call mvn package jboss:redeploy -DskipTests
IF %ERRORLEVEL% NEQ 0 exit -1
@ping 127.0.0.1 -n 5 -w 1000 > nul
call mvn surefire:test
IF %ERRORLEVEL% NEQ 0 exit -1
