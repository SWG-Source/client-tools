@echo off
rem Starts the 64-bit SWG Source client.
rem This file belongs in your SWG Source client folder, next to SwgClient_r.exe.
setlocal
cd /d "%~dp0"

if not exist "x64\SwgClient_r.exe" (
    echo The x64 folder is missing. Unzip the whole 64-bit client package into
    echo your SWG Source client folder, next to SwgClient_r.exe.
    pause
    exit /b 1
)

if not exist "x64\helper-x86\Mss32.dll" (
    call "x64\setup.bat"
    if errorlevel 1 (
        pause
        exit /b 1
    )
)

start "" "x64\SwgClient_r.exe" -- @x64\x64.cfg
