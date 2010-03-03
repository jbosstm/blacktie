cd ../../
rm -rf dist
mkdir dist
cp blacktie/target/blacktie-*-bin.tar.gz dist
cd dist
tar xfz blacktie-*-bin.tar.gz
cd blacktie*/
./run_all_samples.sh
