# Microsoft Developer Studio Project File - Name="ui" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ui - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ui.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ui.mak" CFG="ui - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ui - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ui - Win32 Optimized" (based on "Win32 (x86) Static Library")
!MESSAGE "ui - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "ui"
# PROP Scc_LocalPath "..\.."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ui - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\..\..\compile\win32\ui\Release"
# PROP Intermediate_Dir "..\..\..\..\..\..\compile\win32\ui\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W4 /WX /GR /GX /Zd /O2 /I "..\..\..\..\..\ours\library\unicode\include" /I "..\..\..\directx9/include" /I "..\..\..\stlport453\stlport" /I "..\..\include\\" /D "NDEBUG" /D "_WINDOWS" /D "WIN32" /D "_MBCS" /D "_LIB" /Yu"_precompile.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ui - Win32 Optimized"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Optimized"
# PROP BASE Intermediate_Dir "Optimized"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\..\..\compile\win32\ui\Optimized"
# PROP Intermediate_Dir "..\..\..\..\..\..\compile\win32\ui\Optimized"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MTd /W4 /WX /Gm /GR /GX /Zi /Od /I "..\..\..\..\..\ours\library\unicode\include" /I "..\..\..\directx9/include" /I "..\..\..\stlport453\stlport" /I "..\..\include\\" /I "..\..\..\..\..\..\engine\shared\library\sharedSynchronization\include\public" /I "..\..\..\..\..\..\engine\shared\library\sharedDebug\include\public" /I "..\..\..\..\..\..\engine\shared\library\sharedFoundation\include\public" /I "..\..\..\..\..\..\engine\shared\library\sharedFoundationTypes\include\public" /I "..\..\..\..\..\..\engine\shared\library\sharedMemoryManager\include\public" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "_MBCS" /D "_LIB" /Yu"_precompile.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ui - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\..\..\compile\win32\ui\Debug"
# PROP Intermediate_Dir "..\..\..\..\..\..\compile\win32\ui\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MTd /W4 /WX /GR /GX /Zd /Od /Ob1 /I "..\..\..\..\..\ours\library\unicode\include" /I "..\..\..\directx9/include" /I "..\..\..\stlport453\stlport" /I "..\..\include\\" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "_MBCS" /D "_LIB" /Yu"_precompile.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "ui - Win32 Release"
# Name "ui - Win32 Optimized"
# Name "ui - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\win32\_precompile.cpp
# ADD CPP /Yc
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\SetupUi.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIBaseObject.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\boundary\UIBoundary.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\boundary\UIBoundaryPolygon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIButtonStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UICanvas.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UICanvasGenerator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UICheckbox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UICheckboxStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIClipboard.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIClock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIColorEffector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIComboBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIComposite.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UICursor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UICursorInterface.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UICursorSet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIData.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIDataSource.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIDataSourceBase.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIDataSourceContainer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIDeformer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIDeformerHUD.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIDeformerRotate.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIDeformerWave.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIDropdownbox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIDropdownboxStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIEffector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIEllipse.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIEventCallback.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIFontCharacter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIGBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIGBuffer_Command.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIGrid.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIGridStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\image\UIImageFragment.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\image\UIImageFrame.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIImageStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIListbox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIListboxStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIListStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UILoader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\loader\UILoaderSetup.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UILoaderToken.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UILocalizedStringFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UILocationEffector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UILowerString.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UiMemoryBlockManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIMessage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UINameSpace.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UINotification.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIOpacityEffector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIPacking.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIPage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UIPalette.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UIPaletteRegistrySetup.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIPie.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIPieStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIPopupMenu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIPopupMenuStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIProgressbar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIProgressbarStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\property\UIPropertyCategories.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\property\UIPropertyDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\property\UIPropertyRegister.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\property\UIPropertyRegisterEntry.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\property\UIPropertyValue.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIRadialMenu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIRadialMenuStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIRectangleStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UIRenderHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UiReport.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIRotationEffector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIRunner.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UISaver.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIScriptEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIScrollbar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIScrollLocationEffector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UISizeEffector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UISliderbar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UISliderbarStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UISliderbase.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UISliderplane.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UISliderplaneStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UISmartPointer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIString.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UISystemDependancies.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITabbedPane.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITabbedPaneStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\table\UITable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\table\UITableHeader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\table\UITableModel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\table\UITableModelDefault.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITabSet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITabSetStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITemplate.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITemplateCache.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIText.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITextBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITextboxStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITextStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITextStyleManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UITextStyleWrappedText.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITooltipStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UITreeView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UITreeView_DataNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UITreeViewStyle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITypes.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIUndo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIUnknown.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UIVersion.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIVolumePage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIWatcher.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIWidget.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\boundary\UIWidgetBoundaries.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIWidgetRectangleStyles.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIWidgetStyle.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "boundary"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\shared\boundary\UIBoundary.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\boundary\UIBoundaryPolygon.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\boundary\UIWidgetBoundaries.h
# End Source File
# End Group
# Begin Group "table"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\shared\table\UITable.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\table\UITableHeader.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\table\UITableModel.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\table\UITableModelDefault.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\src\win32\_precompile.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\SetupUi.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UI3DView.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UIActionListener.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIBaseObject.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIButton.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIButtonStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UICanvas.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UICanvasGenerator.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UICheckbox.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UICheckboxStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIClipboard.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIClock.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIColorEffector.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIComboBox.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIComposite.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UICursor.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UICursorInterface.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UICursorSet.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIData.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIDataSource.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIDataSourceBase.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIDataSourceContainer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIDeformer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIDeformerHUD.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIDeformerRotate.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIDeformerWave.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIDropdownbox.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIDropdownboxStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIEffector.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIEllipse.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIEventCallback.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIFontCharacter.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIGBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIGBuffer_Command.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIGrid.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIGridStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIImage.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\image\UIImageFragment.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\image\UIImageFrame.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIImageStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIIMEManager.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIListbox.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIListboxStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIListStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UILoader.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\loader\UILoaderExtension.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\loader\UILoaderSetup.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UILoaderToken.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UILocalizedStringFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UILocationEffector.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UILowerString.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIManager.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UiMemoryBlockManager.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UiMemoryBlockManagerMacros.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIMessage.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIMessageModifierData.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UINamespace.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UINotification.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UINullIMEManager.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIOpacityEffector.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIOutputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIPacking.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIPage.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UIPalette.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIPie.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIPieStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIPopupMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIPopupMenuStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIProgressbar.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIProgressbarStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\property\UIPropertyCategories.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\property\UIPropertyDescriptor.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\property\UIPropertyRegister.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\property\UIPropertyRegisterEntry.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\property\UIPropertyTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\property\UIPropertyValue.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIRadialMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIRadialMenuStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIRectangleStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UIRenderHelper.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UiReport.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIRotationEffector.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIRunner.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UISaver.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIScriptEngine.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIScrollbar.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIScrollLocationEffector.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UISizeEffector.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UISliderbar.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UISliderbarStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UISliderbase.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UISliderplane.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UISliderplaneStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UISmartPointer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UISoundCanvas.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\loader\UIStandardLoader.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIStlFwd.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIString.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UISystemDependancies.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITabbedPane.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITabbedPaneStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITabSet.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITabSetStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITemplate.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITemplateCache.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIText.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITextbox.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITextboxStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITextStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UITextStyleManager.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UITextStyleWrappedText.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITooltipStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UITreeView.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UITreeView_DataNode.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UITreeViewStyle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UITypeID.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UITypes.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIUndo.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIUnknown.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\core\UIVersion.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIVolumePage.h
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\UIWatcher.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIWidget.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIWidgetRectangleStyles.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\UIWidgetStyle.h
# End Source File
# End Group
# Begin Group "Definition Files"

# PROP Default_Filter "*.def"
# Begin Source File

SOURCE=..\..\src\shared\table\UITableFlags.def
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\src\shared\table\UITableTypes.def
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\project.lnt
# End Source File
# End Target
# End Project
