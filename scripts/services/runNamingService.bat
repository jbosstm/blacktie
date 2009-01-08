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

md %ATMIBROKER_BIN_DIR%\work
md %ATMIBROKER_BIN_DIR%\work\namingServiceData

@REM TAO NON PERSISTENT
@REM %TAO_ROOT%\orbsvcs\Naming_Service\Naming_Service -o %ATMIBROKER_BIN_DIR%\work\namingService.ior 

@REM TAO PERSISTENT
@REM %TAO_ROOT%\orbsvcs\Naming_Service\Naming_Service -o %ATMIBROKER_BIN_DIR%\work\namingService.ior -m 1 -u %ATMIBROKER_BIN_DIR%\work\namingServiceData

@REM TAO PERSISTENT ENDPOINT
%TAO_ROOT%\orbsvcs\Naming_Service\Naming_Service -o %ATMIBROKER_BIN_DIR%\work\namingService.ior -m 1 -u %ATMIBROKER_BIN_DIR%\work\namingServiceData -ORBEndPoint iiop://localhost:3528

@REM JACORB
@rem cd %JACORB_HOME%\bin
@rem ns.bat -DOAPort=3528
@rem NOT USED?  %ATMIBROKER_BIN_DIR%\work\namingService.ior

@REM RUNNING WITHIN JBOSS
@rem exit
