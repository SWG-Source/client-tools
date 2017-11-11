@echo off

p4 edit libs/...

REM debug

copy VChatAPI\projects\VChat\lib\debug\* libs\Win32-Debug
copy VChatAPI\utils2.0\lib\debug\* libs\Win32-Debug

REM releae

copy VChatAPI\projects\VChat\lib\release\* libs\Win32-Release
copy VChatAPI\utils2.0\lib\release\* libs\Win32-Release

p4 revert -a libs/...
