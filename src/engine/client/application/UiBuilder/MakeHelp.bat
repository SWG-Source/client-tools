@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by UIBUILDER.HPJ. >"hlp\UiBuilder.hm"
echo. >>"hlp\UiBuilder.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\UiBuilder.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\UiBuilder.hm"
echo. >>"hlp\UiBuilder.hm"
echo // Prompts (IDP_*) >>"hlp\UiBuilder.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\UiBuilder.hm"
echo. >>"hlp\UiBuilder.hm"
echo // Resources (IDR_*) >>"hlp\UiBuilder.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\UiBuilder.hm"
echo. >>"hlp\UiBuilder.hm"
echo // Dialogs (IDD_*) >>"hlp\UiBuilder.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\UiBuilder.hm"
echo. >>"hlp\UiBuilder.hm"
echo // Frame Controls (IDW_*) >>"hlp\UiBuilder.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\UiBuilder.hm"
REM -- Make help for Project UIBUILDER


echo Building Win32 Help files
start /wait hcw /C /E /M "hlp\UiBuilder.hpj"
if errorlevel 1 goto :Error
if not exist "hlp\UiBuilder.hlp" goto :Error
if not exist "hlp\UiBuilder.cnt" goto :Error
echo.
if exist Debug\nul copy "hlp\UiBuilder.hlp" Debug
if exist Debug\nul copy "hlp\UiBuilder.cnt" Debug
if exist Release\nul copy "hlp\UiBuilder.hlp" Release
if exist Release\nul copy "hlp\UiBuilder.cnt" Release
echo.
goto :done

:Error
echo hlp\UiBuilder.hpj(1) : error: Problem encountered creating help file

:done
echo.
