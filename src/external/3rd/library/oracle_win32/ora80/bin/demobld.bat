@echo off
setlocal
if (%2)==() goto usage
sqlplus %1/%2 @%ORACLE_HOME%\sqlplus\demo\demobld.sql
goto exit
:usage
echo Usage: demobld userid passwd
:end
endlocal
