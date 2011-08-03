# ALLOW JOBS TO BE BACKGROUNDED
set -m

echo "Running all quickstarts"

for i in `find $BLACKTIE_HOME/quickstarts/ -name run.sh`
do
    . $i
    if [ "$?" != "0" ]; then
	    exit -1
    fi
done

echo "All quickstarts ran OK"
