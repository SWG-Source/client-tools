@echo off
::if  not exist "%QTDIR%\bin\progress.exe" (
	(
	"%QTDIR%\misc\findpath\findpath" mingw32-make.exe >nul
	if errorlevel 1 (
		"%QTDIR%\misc\findpath\findpath" nmake.exe >nul
		if errorlevel 1 (
			rem Borland 
		) else (
			cd misc\progress
			nmake -f Makefile.win32-msvc
			cd ..\..
		)
	) else (
		cd misc\progress
		mingw32-make -f Makefile.win32-g++
		cd ..\..
	)
)

if   exist "%QTDIR%\misc\progress\progress.exe" (
	copy misc\progress\progress.exe bin
)

if  exist "%QTDIR%\bin\progress.exe" (
	echo.
	echo Starting progress window
	start progress.exe 
	echo.
)

configure.bat %*
