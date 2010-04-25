@echo off
java -Dlog4j.configuration=log4j.xml org.jboss.blacktie.btadmin.BTAdmin %*
