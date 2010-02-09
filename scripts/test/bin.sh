cd ../../
set -m
rm -rf tmp
mkdir tmp
cp blacktie/target/blacktie-*-bin.tar.gz tmp
cd tmp
tar xfz blacktie-*-bin.tar.gz
cd blacktie*/
sed -i "s=REPLACE_WITH_INSTALL_LOCATION=$VAR=g" setenv.sh
. setenv.sh
cd $BLACKTIE_BIN_DIR/examples/xatmi/fooapp
$BLACKTIE_BIN_DIR/bin/generate_server.sh -Dservice.names=BAR -Dserver.includes=BarService.c
./server -c linux -i 1&
$BLACKTIE_BIN_DIR/bin/generate_client.sh -Dclient.includes=client.c
./client
cd $BLACKTIE_BIN_DIR/examples/jab
echo hello | mvn test
cd $BLACKTIE_BIN_DIR/examples/admin/jmx
echo '0
0
0
0
1' | mvn test
fg
cd $BLACKTIE_BIN_DIR/examples/xatmi/fooapp
./server -c linux -i 1&
cd $BLACKTIE_BIN_DIR/examples/admin/xatmi
$BLACKTIE_BIN_DIR/bin/generate_client.sh -Dclient.includes=client.c
echo '0
0
0
0
1' | ./client
fg
cd $BLACKTIE_BIN_DIR/examples/mdb
mvn install
