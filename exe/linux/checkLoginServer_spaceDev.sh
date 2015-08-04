# ! /bin/sh
#

cd /swg/swg/stage/exe/linux
STATUS=`release/LoginPing -- @sdkswg-15/servercommon.cfg -s SharedNetwork useTcp=false`
if [ "$STATUS" != "Ok" ]; then
	/swg/swg/stage/exe/linux/startLoginServer_spaceDev.sh &
	echo "LoginServer not responding"
fi
