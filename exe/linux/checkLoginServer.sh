# ! /bin/sh
#

cd /swg/swg/live/exe/linux
STATUS=`release/LoginPing -- -s SharedNetwork useTcp=false`
if [ "$STATUS" != "Ok" ]; then
	/swg/swg/live/exe/linux/startLoginServer.sh &
	echo "LoginServer not responding"
fi
