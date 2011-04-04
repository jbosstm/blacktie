# ALLOW JOBS TO BE BACKGROUNDED
set -m

# RUN THE FOOAPP SERVER
echo "Example: Running fooapp"
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
