# ! /bin/sh
#

cd /swg/swg/stage/exe/linux
STATUS=`release/LoginPing -- @sdkswg-17/servercommon.cfg -s SharedNetwork useTcp=false`
if [ "$STATUS" != "Ok" ]; then
	/swg/swg/stage/exe/linux/startLoginServer_spaceBeta.sh &
	echo "LoginServer not responding"
fi
