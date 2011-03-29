# ALLOW JOBS TO BE BACKGROUNDED
set -m

echo "Running all samples"

for i in `find $BLACKTIE_HOME/examples/ -name run.sh`
do
    . $i
    if [ "$?" != "0" ]; then
	    exit -1
    fi
done

echo "All samples ran OK"
