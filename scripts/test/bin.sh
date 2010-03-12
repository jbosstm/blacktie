cd ../../
rm -rf dist
mkdir dist
cp blacktie/target/blacktie-*-bin.tar.gz dist
cd dist
tar xfz blacktie-*-bin.tar.gz
cd blacktie*/

# MAKE SURE THE ENVIRONMENT VARIABLES ARE SET
export VAR=`pwd`
sed -i "s=REPLACE_WITH_INSTALL_LOCATION=$VAR=g" setenv.sh
. setenv.sh

./run_all_samples.sh
