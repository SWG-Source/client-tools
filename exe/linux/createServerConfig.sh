#!/bin/bash

#this creates taskmanager.cfg, loginServer.cfg, and taskmanager.rc from the template files.  
#set your CLUSTERNAME environment variable to your desired cluster name.  If none exists, it will use your username
#export CLUSTERNAME=MyClusterName

FILEPATH=../../exe/linux
CLUSTER=${CLUSTERNAME:=$USER}
BRANCH=`pwd |sed -e "/^.*swg\//s///" -e "/\/.*$/s///"`
if [ "$BRANCH" == "current" ]; then
	DBUSER=$USER
else
	DBUSER=${USER}_$BRANCH
fi
HOSTIP=`/sbin/ifconfig |grep 'Bcast' |sed -e 's/^[^:]*://' -e 's/ .*$//'`

FILES="localOptions.cfg loginServer.cfg logServerTargets.cfg serverNetwork.cfg taskmanager.cfg taskmanager.rc"

for FILE in $FILES; do
	sed -e s/HOSTNAME/$HOSTNAME/ -e s/USERNAME/$USER/ -e s/CLUSTERNAME/$CLUSTER/ -e s/DBUSER/$DBUSER/ -e s/HOSTIP/$HOSTIP/ $FILEPATH/_$FILE > $FILE
	chmod 644 $FILE
done

