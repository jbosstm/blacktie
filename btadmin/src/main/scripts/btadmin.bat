java -Dlog4j.configuration=log4j.xml org.jboss.blacktie.btadmin.BTAdmin %*
IF %ERRORLEVEL% NEQ 0 exit -1