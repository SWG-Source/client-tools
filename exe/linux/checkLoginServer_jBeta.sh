# ! /bin/sh
#

cd /swg/swg/live/exe/linux
STATUS=`release/LoginPing -- @jpeswg-01/servercommon.cfg -s SharedNetwork useTcp=false`
if [ "$STATUS" != "Ok" ]; then
	/swg/swg/live/exe/linux/startLoginServer_jBeta.sh &
	echo "LoginServer not responding"
fi
