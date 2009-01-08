@echo off
rem
rem JBoss, Home of Professional Open Source
rem Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
rem by the @authors tag. All rights reserved.
rem See the copyright.txt in the distribution for a
rem full listing of individual contributors.
rem This copyrighted material is made available to anyone wishing to use,
rem modify, copy, or redistribute it subject to the terms and conditions
rem of the GNU Lesser General Public License, v. 2.1.
rem This program is distributed in the hope that it will be useful, but WITHOUT A
rem WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
rem PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
rem You should have received a copy of the GNU Lesser General Public License,
rem v.2.1 along with this distribution; if not, write to the Free Software
rem Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
rem MA  02110-1301, USA.
rem

set ATMIBROKER_BIN_DIR=REPLACE_WITH_INSTALL_LOCATION
set ACE_ROOT=REPLACE_WITH_PATH_TO_ACE_wrappers
set LOG4CXX_ROOT=REPLACE_WITH_PATH_TO_BUILT_LOG4CXX_ROOT
set JBOSSTS_HOME=REPLACE_WITH_PATH_TO_JBOSSTS_HOME
set EXPAT_ROOT=REPLACE_WITH_PATH_TO_EXPAT
set CPPUNIT_ROOT=REPLACE_WITH_PATH_TO_CPPUNIT

set PATH=%PATH%;%ATMIBROKER_BIN_DIR%\bin
set PATH=%PATH%;%ATMIBROKER_BIN_DIR%\libTao
set PATH=%PATH%;%ACE_ROOT%\lib
set PATH=%PATH%;%LOG4CXX_ROOT%\target\debug\shared
set PATH=%PATH%;%EXPAT_ROOT%\Bin
set PATH=%PATH%;%CPPUNIT_ROOT%\lib

setlocal ENABLEDELAYEDEXPANSION
FOR /R codeGeneration %%G IN (*.jar) DO set CLASSPATH=!CLASSPATH!;%%G
FOR /R connectors\jab %%G IN (*.jar) DO set CLASSPATH=!CLASSPATH!;%%G
endlocal & set CLASSPATH=%CLASSPATH%
set CLASSPATH=%CLASSPATH%;%ATMIBROKER_BIN_DIR%\bin
set CLASSPATH=%CLASSPATH%;.
