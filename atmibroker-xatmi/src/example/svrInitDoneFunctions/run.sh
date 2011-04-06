# ALLOW JOBS TO BE BACKGROUNDED
set -m

echo "Example: Running svrInitDoneFunctions"

cd $BLACKTIE_HOME/examples/xatmi/svrInitDoneFunctions

# RUN THE FOOAPP SERVER
generate_server -Dservice.names=BAR -Dserver.includes=BarService.c,SvrInit.c
if [ "$?" != "0" ]; then
	exit -1
fi
btadmin startup
if [ "$?" != "0" ]; then
	exit -1
fi

btadmin shutdown
if [ "$?" != "0" ]; then
	exit -1
fi
