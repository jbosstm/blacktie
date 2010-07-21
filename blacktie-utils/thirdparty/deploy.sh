export bpa=centos55x32

for i in ace-5.7.6 cppunit-1.12 expat-2.0.1 xercesc-3.0.1
do
mvn deploy:deploy-file -Durl=https://repository.jboss.org/nexus/service/local/staging/deploy/maven2/ -DrepositoryId=jboss-releases -Dfile=`pwd`/build/$i-$bpa.zip -DpomFile=poms/$1.pom -Dpackaging=zip -Dclassifier=$bpa
done


for i in apr-1-1.3.3 log4cxx-902683
do
mvn deploy:deploy-file -Durl=https://repository.jboss.org/nexus/service/local/staging/deploy/maven2/ -DrepositoryId=jboss-releases -Dfile=`pwd`/build/$i-$bpa.jar -DpomFile=poms/$1.pom -Dpackaging=jar -Dclassifier=$bpa
done
