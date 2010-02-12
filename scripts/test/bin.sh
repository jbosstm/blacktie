# ALLOW JOBS TO BE BACKGROUNDED
set -m

# SET UP THE DISTRIBUTION
cd ../../
rm -rf tmp
mkdir tmp
cp blacktie/target/blacktie-*-bin.tar.gz tmp
cd tmp
tar xfz blacktie-*-bin.tar.gz
cd blacktie*/
export VAR=`pwd`
sed -i "s=REPLACE_WITH_INSTALL_LOCATION=$VAR=g" setenv.sh

# MAKE SURE THE ENVIRONMENT VARIABLES ARE SET
. setenv.sh

# RUN THE FOOAPP SERVER
cd $BLACKTIE_BIN_DIR/examples/xatmi/fooapp
$BLACKTIE_BIN_DIR/bin/generate_server.sh -Dservice.names=BAR -Dserver.includes=BarService.c
if [ "$?" != "0" ]; then
	exit -1
fi
./server -c linux -i 1&

# RUN THE C CLIENT
$BLACKTIE_BIN_DIR/bin/generate_client.sh -Dclient.includes=client.c
./client
if [ "$?" != "0" ]; then
	exit -1
fi

# RUN THE JAVA CLIENT
cd $BLACKTIE_BIN_DIR/examples/jab
echo hello | mvn test
if [ "$?" != "0" ]; then
	exit -1
fi

# RUN THE ADMIN JMX CLIENT
cd $BLACKTIE_BIN_DIR/examples/admin/jmx
echo '0
0
0
0
1' | mvn test
if [ "$?" != "0" ]; then
	exit -1
fi
# PICK UP THE CLOSING SERVER
fg

# RUN THE FOOAPP SERVER AGAIN
cd $BLACKTIE_BIN_DIR/examples/xatmi/fooapp
./server -c linux -i 1&

# SHUTDOWN THE SERVER RUNNING THE XATMI ADMIN CLIENT
cd $BLACKTIE_BIN_DIR/examples/admin/xatmi
$BLACKTIE_BIN_DIR/bin/generate_client.sh -Dclient.includes=client.c
echo '0
0
0
0
1' | ./client
# PICK UP THE CLOSING SERVER
fg

# RUN THE MDB EXAMPLE
cd $BLACKTIE_BIN_DIR/examples/mdb
mvn install
if [ "$?" != "0" ]; then
	exit -1
fi

# RUN THE SECURE SERVER
cd $BLACKTIE_BIN_DIR/examples/security
$BLACKTIE_BIN_DIR/bin/generate_server.sh -Dservice.names=SECURE -Dserver.includes=BarService.c
if [ "$?" != "0" ]; then
	exit -1
fi
export BLACKTIE_CONFIGURATION_DIR=serv
./server -c linux -i 1 secure&
unset BLACKTIE_CONFIGURATION_DIR

# RUN THE "guest" USER CLIENT
$BLACKTIE_BIN_DIR/bin/generate_client.sh -Dclient.includes=client.c
export BLACKTIE_CONFIGURATION_DIR=guest
./client
# This test is expected to fail so make sure the exit status was not 0
if [ "$?" == "0" ]; then
	exit -1
fi
unset BLACKTIE_CONFIGURATION_DIR

# RUN THE "dynsub" USER CLIENT
export BLACKTIE_CONFIGURATION_DIR=dynsub
./client
if [ "$?" != "0" ]; then
	exit -1
fi
unset BLACKTIE_CONFIGURATION_DIR

# SHUTDOWN THE SERVER RUNNING THE XATMI ADMIN CLIENT
cd $BLACKTIE_BIN_DIR/examples/admin/xatmi
$BLACKTIE_BIN_DIR/bin/generate_client.sh -Dclient.includes=client.c
unset BLACKTIE_CONFIGURATION_DIR
echo '0
0
0
0
1' | ./client
# PICK UP THE CLOSING SERVER
fg

