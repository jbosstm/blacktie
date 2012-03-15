cd %WORKSPACE%
call ant -f scripts/hudson/initializeJBoss.xml -Dbasedir=. initializeDatabase initializeJBoss -debug
IF %ERRORLEVEL% NEQ 0 exit -1

cd %WORKSPACE%\jboss-as-7.1.1.Final\bin\
call add-user admin password --silent=true
IF %ERRORLEVEL% NEQ 0 exit -1
call add-user guest password -a --silent=true
IF %ERRORLEVEL% NEQ 0 exit -1
call add-user dynsub password -a --silent=true
IF %ERRORLEVEL% NEQ 0 exit -1

cd %WORKSPACE%

rem INITIALZE JBOSSESB
rem copy %WORKSPACE%\scripts\hudson\hornetq\jboss-as-hornetq-int.jar %WORKSPACE%\jboss-5.1.0.GA\common\lib
rem copy %WORKSPACE%\scripts\hudson\hornetq\hornetq-deployers-jboss-beans.xml %WORKSPACE%\jboss-5.1.0.GA\server\all-with-hornetq\deployers
rem copy %WORKSPACE%\jbossesb-4.9\install\deployment.properties-example %WORKSPACE%\jbossesb-4.9\install\deployment.properties
rem call ant configureESB -DWORKSPACE=%WORKSPACE:\=/%
rem IF %ERRORLEVEL% NEQ 0 exit -1
rem cd %WORKSPACE%\jbossesb-4.9\install
rem call ant deploy
rem IF %ERRORLEVEL% NEQ 0 exit -1
rem cd %WORKSPACE%
