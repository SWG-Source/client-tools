@echo off
setlocal
if (%2)==() goto usage
sqlplus %1/%2 @%ORACLE_HOME%\sqlplus\demo\demodrop.sql
goto exit
:usage
echo Usage: demodrop userid passwd
:end
endlocal
