cd ../../
set -m
rm -rf tmp
mkdir tmp
cp blacktie/target/blacktie-*-bin.tar.gz tmp
cd tmp
tar xfz blacktie-*-bin.tar.gz
cd blacktie*/
export VAR=`pwd`
sed -i "s=REPLACE_WITH_INSTALL_LOCATION=$VAR=g" setenv.sh

. setenv.sh

# RUN THE SERVER
cd $BLACKTIE_BIN_DIR/examples/xatmi/fooapp
$BLACKTIE_BIN_DIR/bin/generate_server.sh -Dservice.names=BAR -Dserver.includes=BarService.c
if [ "$?" != "0" ]; then
	exit $?
fi
./server -c linux -i 1&

# RUN THE C CLIENT
$BLACKTIE_BIN_DIR/bin/generate_client.sh -Dclient.includes=client.c
./client
if [ "$?" != "0" ]; then
	exit $?
fi

# RUN THE JAVA CLIENT
cd $BLACKTIE_BIN_DIR/examples/jab
echo hello | mvn test
if [ "$?" != "0" ]; then
	exit $?
fi

# RUN THE ADMIN JMX CLIENT
cd $BLACKTIE_BIN_DIR/examples/admin/jmx
echo '0
0
0
0
1' | mvn test

# PICK UP THE CLOSING SERVER
fg

# RUN THE SERVER AGAIN
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
fg

# RUN THE MDB EXAMPLE
cd $BLACKTIE_BIN_DIR/examples/mdb
mvn install
if [ "$?" != "0" ]; then
	exit $?
fi
