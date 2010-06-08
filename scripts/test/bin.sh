export DIST=dist

rm -rf $DIST
mkdir $DIST
tar xfz blacktie/target/blacktie-*-bin.tar.gz -C $DIST
cd $DIST/blacktie*/

# MAKE SURE THE ENVIRONMENT VARIABLES ARE SET
export HOSTNAME_TO_USE=`hostname`
export REPLACE_WITH_INSTALL_LOCATION=`pwd`
sed -i "s=REPLACE_WITH_INSTALL_LOCATION=$REPLACE_WITH_INSTALL_LOCATION=g" setenv.sh
for i in `find . -name btconfig.xml`; do sed -i "s=REPLACE_WITH_INSTALL_LOCATION=$REPLACE_WITH_INSTALL_LOCATION=g" $i; done
for i in `find . -name btconfig.xml`; do sed -i "s=REPLACE_WITH_HOSTNAME=$HOSTNAME_TO_USE=g" $i; done
for i in `find . -name pom.xml`; do sed -i "s=REPLACE_WITH_HOSTNAME=$JBOSSAS_IP_ADDR=g" $i; done
for i in `find . -name btconfig.xml`; do sed -i "s=$HOSTNAME_TO_USE:3528=$JBOSSAS_IP_ADDR:3528=g" $i; done
for i in `find . -name btconfig.xml`; do sed -i "s=$HOSTNAME_TO_USE:109=$JBOSSAS_IP_ADDR:109=g" $i; done

for i in `find . -name btconfig.xml`; do sed -i "s/<!-- DB2 support -->/<!--/g" $i; done
for i in `find . -name btconfig.xml`; do sed -i "s/<!-- END DB2 support -->/-->/g" $i; done

for i in `find . -name btconfig.xml`; do sed -i "s/SqlNet=blacktie+DB=blacktie/SqlNet=orcl112+DB=ORCL.NCL/g" $i; done
for i in `find . -name request.c`; do sed -i "s/blacktie/ORCL.NCL/g" $i; done
