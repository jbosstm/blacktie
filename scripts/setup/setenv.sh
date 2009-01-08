#
# JBoss, Home of Professional Open Source
# Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
# by the @authors tag. All rights reserved.
# See the copyright.txt in the distribution for a
# full listing of individual contributors.
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU Lesser General Public License, v. 2.1.
# This program is distributed in the hope that it will be useful, but WITHOUT A
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
# You should have received a copy of the GNU Lesser General Public License,
# v.2.1 along with this distribution; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.
#
ATMIBROKER_BIN_DIR=REPLACE_WITH_INSTALL_LOCATION
ACE_ROOT=REPLACE_WITH_PATH_TO_ACE_wrappers
TAO_ROOT=REPLACE_WITH_PATH_TO_TAO
LOG4CXX_ROOT=REPLACE_WITH_PATH_TO_BUILT_LOG4CXX_ROOT
JBOSSTS_HOME=REPLACE_WITH_PATH_TO_JBOSSTS_HOME
export ATMIBROKER_BIN_DIR
export ACE_ROOT
export TAO_ROOT
export LOG4CXX_ROOT
export JBOSSTS_HOME
 
PATH=$PATH:$ATMIBROKER_BIN_DIR/bin
PATH=$PATH:$ATMIBROKER_BIN_DIR/libTao
PATH=$PATH:$ACE_ROOT/lib
PATH=$PATH:$LOG4CXX_ROOT/target/debug/shared
export PATH

CLASSPATH=
for i in $ATMIBROKER_BIN_DIR/codeGeneration/*.jar $ATMIBROKER_BIN_DIR/connectors/jab/*.jar
do
CLASSPATH="$CLASSPATH:$i"
done
CLASSPATH="$CLASSPATH:$ATMIBROKER_BIN_DIR/bin"
CLASSPATH="$CLASSPATH:."
export CLASSPATH

LD_LIBRARY_PATH=$ATMIBROKER_BIN_DIR/libTao
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ACE_ROOT/lib
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LOG4CXX_ROOT/target/debug/shared
export LD_LIBRARY_PATH