#
# JBoss, Home of Professional Open Source
# Copyright 2008, Red Hat, Inc., and others contributors as indicated
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
BLACKTIE_BIN_DIR=REPLACE_WITH_INSTALL_LOCATION
export BLACKTIE_BIN_DIR
BLACKTIE_SCHEMA_DIR=$BLACKTIE_BIN_DIR/schemas/xsd
export BLACKTIE_SCHEMA_DIR

PATH=$PATH:$BLACKTIE_BIN_DIR/bin
export PATH

LD_LIBRARY_PATH=$BLACKTIE_BIN_DIR/lib
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
export LD_LIBRARY_PATH

CLASSPATH=
for i in $BLACKTIE_BIN_DIR/codeGeneration/*.jar $BLACKTIE_BIN_DIR/connectors/jab/*.jar
do
CLASSPATH="$CLASSPATH:$i"
done
CLASSPATH="$CLASSPATH:$BLACKTIE_BIN_DIR/bin"
CLASSPATH="$CLASSPATH:."
export CLASSPATH

