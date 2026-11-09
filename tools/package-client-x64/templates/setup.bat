@echo off
rem One-time setup for the 64-bit client. SwgClient_x64.bat runs this for you.
rem
rem Sound and movies still use the original 32-bit Miles and Bink libraries,
rem through a small helper program (helper-x86\miles-host.exe). This copies
rem your client's own copies of those libraries next to the helper. Nothing
rem outside the x64 folder is changed.
setlocal
set "CLIENT=%~dp0.."
set "HELPER=%~dp0helper-x86"

for %%F in (Mss32.dll binkw32.dll) do (
    if not exist "%CLIENT%\%%F" (
        echo Could not find %%F in your SWG Source client folder.
        echo Unzip the 64-bit client package into the client folder, next to SwgClient_r.exe.
        exit /b 1
    )
)
if not exist "%CLIENT%\miles\" (
    echo Could not find the miles folder in your SWG Source client folder.
    exit /b 1
)

echo Setting up sound and movies for the 64-bit client...
copy /y "%CLIENT%\Mss32.dll" "%HELPER%\" >nul || exit /b 1
copy /y "%CLIENT%\binkw32.dll" "%HELPER%\" >nul || exit /b 1
xcopy /e /i /y /q "%CLIENT%\miles" "%HELPER%\miles" >nul || exit /b 1
echo Done.
exit /b 0
