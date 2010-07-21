echo "Making ACE+TAO-$ACE_VER"
rm -rf ACE+TAO-$ACE_VER
mkdir ACE+TAO-$ACE_VER
wget http://download.dre.vanderbilt.edu/previous_versions/ACE+TAO-$ACE_VER.tar.gz
tar xfz ACE+TAO-$ACE_VER.tar.gz -C ACE+TAO-$ACE_VER
export ACE_ROOT=/home/hudson/blacktie/utils/ACE+TAO-$ACE_VER/ACE_wrappers
export TAO_ROOT=/home/hudson/blacktie/utils/ACE+TAO-$ACE_VER/ACE_wrappers/TAO
export LD_LIBRARY_PATH=$ACE_ROOT/lib
cd $ACE_ROOT
echo "#include \"ace/config-linux.h\"" > $ACE_ROOT/ace/config.h
echo "include \$(ACE_ROOT)/include/makeinclude/platform_linux.GNU" > $ACE_ROOT/include/makeinclude/platform_macros.GNU
for i in ace apps/gperf/src ACEXML $TAO_ROOT/TAO_IDL $TAO_ROOT/tao $TAO_ROOT/orbsvcs/ImplRepo_Service $TAO_ROOT/orbsvcs/orbsvcs
do
(cd $i && make)
done
