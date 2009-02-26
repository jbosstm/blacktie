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

rem THIS IS NO LONGER REQUIRED IF YOU ARE USING THE JBOSS AS CONTAINER 

set JACORB_HOME=REPLACE_WITH_PATH_TO_JACORB_HOME
set JBOSSTS_HOME=REPLACE_WITH_PATH_TO_JBOSSTS_HOME
set PATH=%PATH%;%JACORB_HOME%\bin

start %JACORB_HOME%\bin\ns.bat -DOAPort=3528
PING 1.1.1.1 -n 1 -w 1000 >NUL
start %JBOSSTS_HOME%\bin\start-recovery-manager.bat
start %JBOSSTS_HOME%\bin\start-transaction-service.bat
