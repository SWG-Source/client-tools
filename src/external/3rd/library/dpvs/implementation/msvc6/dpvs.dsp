# Microsoft Developer Studio Project File - Name="dpvs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=dpvs - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dpvs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dpvs.mak" CFG="dpvs - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dpvs - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dpvs - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dpvs - Win32 IntelCPP" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/dpvs/implementation/msvc6", VPMAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dpvs - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "dpvs___Win32_Release"
# PROP BASE Intermediate_Dir "dpvs___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\lib\win32-x86"
# PROP Intermediate_Dir "release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W4 /Zi /O2 /I "..\..\interface" /I "..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DPVS_DLL" /D "DPVS_BUILD_LIBRARY" /FAs /FD /QIfdiv- /QI0f- /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /map /debug /machine:I386 /out:"..\..\bin\win32-x86\dpvs.dll" /libpath:"..\flexlm\release" /libpath:"..\..\..\remotedebugger\library\\" /libpath:"..\..\..\hl\lib\win32-x86-msvc"
# SUBTRACT LINK32 /profile /nodefaultlib

!ELSEIF  "$(CFG)" == "dpvs - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "dpvs___Win32_Debug"
# PROP BASE Intermediate_Dir "dpvs___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\lib\win32-x86"
# PROP Intermediate_Dir "debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W4 /Zi /Od /I "..\..\interface" /I "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DPVS_DLL" /D "DPVS_BUILD_LIBRARY" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /dll /incremental:no /map /debug /debugtype:both /machine:I386 /out:"..\..\bin\win32-x86\dpvsd.dll" /pdbtype:sept /libpath:"..\remotedebugger\lib\\" /libpath:"..\hl\lib\win32-x86-msvc\\"

!ELSEIF  "$(CFG)" == "dpvs - Win32 IntelCPP"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "dpvs___Win32_IntelCPP"
# PROP BASE Intermediate_Dir "dpvs___Win32_IntelCPP"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "dpvs___Win32_IntelCPP"
# PROP Intermediate_Dir "dpvs___Win32_IntelCPP"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /W4 /Zi /O2 /I "..\..\interface" /I "..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DPVS_DLL" /D "DPVS_BUILD_LIBRARY" /FAs /YX /FD /QIfdiv- /QI0f- /c
# ADD CPP /nologo /G6 /W4 /O2 /I "..\..\interface" /I "..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "DPVS_DLL" /D "DPVS_BUILD_LIBRARY" /D "_USE_INTEL_COMPILER" /FAs /YX /FD /O3 /QIfdiv- /QI0f- -Qunroll0 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /map /debug /machine:I386 /out:"..\..\bin\win32-x86\dpvs.dll" /libpath:"..\flexlm\release"
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 /nologo /subsystem:windows /dll /map /machine:I386 /out:"..\..\bin\win32-x86\dpvs.dll" /libpath:"..\flexlm\release"
# SUBTRACT LINK32 /pdb:none /debug /nodefaultlib

!ENDIF 

# Begin Target

# Name "dpvs - Win32 Release"
# Name "dpvs - Win32 Debug"
# Name "dpvs - Win32 IntelCPP"
# Begin Group "Private Documentation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\docs\dpvsChanges.txt
# End Source File
# Begin Source File

SOURCE=..\docs\dpvsNotes.txt
# End Source File
# Begin Source File

SOURCE=..\docs\dpvsShaft.txt
# End Source File
# Begin Source File

SOURCE=..\docs\dpvsToDo.txt
# End Source File
# End Group
# Begin Group "Private Implementation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\sources\dpvsAABB.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsAABB.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsArray.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsArray.hpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsBaseMath.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsBitMath.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsBitMath.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsBlockBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsBlockBuffer.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsBounds.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsBounds.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsClipPolygon.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsClipPolygon.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsDatabase.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsDatabase.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsDataPasser.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsDataPasser.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsDebug.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsDebug.hpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsEvaluation.hpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsFiller.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsGameCube.cpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsGameCubeDebug.pch
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsGameCubeRelease.pch
# End Source File
# Begin Source File

SOURCE=..\include\dpvsHash.hpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsHashKey.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsHZBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsHZBuffer.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsImpCamera.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsImpCamera.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsImpCell.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsImpCell.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsImpCommander.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsImpCommander.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsImpMeshModel.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsImpMeshModel.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsImpMiscModel.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsImpMiscModel.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsImpModel.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsImpModel.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsImpObject.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsImpObject.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsImpPhysicalPortal.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsImpPhysicalPortal.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsImpRegionOfInfluence.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsImpRegionOfInfluence.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsImpVirtualPortal.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsImpVirtualPortal.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsInstanceCount.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsInstanceCount.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsIntersect.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsIntersect.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsLibraryServices.cpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsMath.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsMath.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsMatrix.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsMatrix.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsMatrixConverter.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsMatrixConverter.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsMemory.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsMemory.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsMemoryPool.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsMemoryPool.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsMesh.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsMesh.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsMT.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsMT.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsNameHash.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsNameHash.hpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsNew.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsOBB.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsOBB.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsOcclusionBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsOcclusionBuffer.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsOcclusionBuffer_Buffer.cpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsOcclusionBuffer_CacheFiller.cpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsOcclusionBuffer_Edges.cpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsOcclusionBuffer_Render.cpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsOcclusionBuffer_Stencil.cpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsOcclusionBuffer_Test.cpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsOcclusionBuffer_ZGradient.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsOcclusionBuffer_ZGradient.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsOcclusionBuffer_ZGradientPS2.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsParallelMath.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsPPCTest.cpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsPrecompDebug.pch
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsPrecompRelease.pch
# End Source File
# Begin Source File

SOURCE=..\include\dpvsPriorityQueue.hpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsPrivateDefs.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsPS2Test.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsPtr.hpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsQWord.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsRandom.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsRandom.hpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsRange.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsRectangle.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsRectangle.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsRecursionSolver.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsRecursionSolver.hpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsRecycler.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsRemoteDebugger.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsRemoteDebugger.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsScratchpad.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsScratchpad.hpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsSet.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsSilhouette.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsSilhouette.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsSilhouetteCache.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsSilhouetteCache.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsSilhouetteCacheCommon.cpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsSilhouetteMath.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsSilhouetteMath.hpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsSort.hpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsSpaceManager.hpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsSphere.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsStatistics.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsStatistics.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsSurface.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsSurface.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsSweepAndPrune.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsSweepAndPrune.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsSystem.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsTempAllocator.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsTempAllocator.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsTempArray.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsTempArray.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsUtility.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsVector.hpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsVersion.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsVersion.rc
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsVisibilityQuery.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsVisibilityQuery.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsVisibilityQuery_Resolve.cpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsVisibilityQuery_Test.cpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsVisibilityQuery_Traverse.cpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsVQData.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsVQData.hpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsWeldHash.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsWindows.cpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsWrapper.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsWrapper.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsWrapperCreate.cpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsWriteQueue.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsWriteQueue.hpp
# End Source File
# Begin Source File

SOURCE=..\sources\dpvsX86.cpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsX86.hpp
# End Source File
# Begin Source File

SOURCE=..\include\dpvsXFirstTraversal.hpp
# End Source File
# End Group
# Begin Group "Public Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\interface\dpvs.hpp
# End Source File
# Begin Source File

SOURCE=..\..\interface\dpvsCamera.hpp
# End Source File
# Begin Source File

SOURCE=..\..\interface\dpvsCell.hpp
# End Source File
# Begin Source File

SOURCE=..\..\interface\dpvsCommander.hpp
# End Source File
# Begin Source File

SOURCE=..\..\interface\dpvsDefs.hpp
# End Source File
# Begin Source File

SOURCE=..\..\interface\dpvsLibrary.hpp
# End Source File
# Begin Source File

SOURCE=..\..\interface\dpvsModel.hpp
# End Source File
# Begin Source File

SOURCE=..\..\interface\dpvsObject.hpp
# End Source File
# Begin Source File

SOURCE=..\..\interface\dpvsReferenceCount.hpp
# End Source File
# Begin Source File

SOURCE=..\..\interface\dpvsUtility.hpp
# End Source File
# End Group
# Begin Group "Public Documentation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\doc\dpvs_online.pdf
# End Source File
# Begin Source File

SOURCE=..\..\doc\dpvsReleaseNotes.txt
# End Source File
# End Group
# Begin Group "Makefiles"

# PROP Default_Filter ""
# Begin Group "Platforms"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\gmake\platforms\aix-rs6000-iva.mk"
# End Source File
# Begin Source File

SOURCE=..\gmake\platforms\gcc.mk
# End Source File
# Begin Source File

SOURCE=..\gmake\platforms\gcn.mk
# End Source File
# Begin Source File

SOURCE="..\gmake\platforms\hpux-hppa-acc.mk"
# End Source File
# Begin Source File

SOURCE="..\gmake\platforms\linux-x86-gcc.mk"
# End Source File
# Begin Source File

SOURCE=..\gmake\platforms\macosx.mk
# End Source File
# Begin Source File

SOURCE=..\gmake\platforms\ps2.mk
# End Source File
# Begin Source File

SOURCE=..\gmake\platforms\sgi.mk
# End Source File
# Begin Source File

SOURCE="..\gmake\platforms\win32-x86-gcc.mk"
# End Source File
# Begin Source File

SOURCE="..\gmake\platforms\win32-x86-kcc.mk"
# End Source File
# Begin Source File

SOURCE="..\gmake\platforms\win32-x86.mk"
# End Source File
# Begin Source File

SOURCE=..\gmake\platforms\xbox.mk
# End Source File
# End Group
# Begin Group "Mak"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\gmake\mak\common.mk
# End Source File
# Begin Source File

SOURCE=..\gmake\mak\rules.mk
# End Source File
# Begin Source File

SOURCE=..\gmake\mak\vis_common.mk
# End Source File
# Begin Source File

SOURCE=..\gmake\mak\vis_demos.mk
# End Source File
# End Group
# Begin Group "Batch Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\build.bat
# End Source File
# Begin Source File

SOURCE=..\..\clean.bat
# End Source File
# Begin Source File

SOURCE="..\..\distribute-flexlm.bat"
# End Source File
# Begin Source File

SOURCE="..\..\distribute-source-copy.bat"
# End Source File
# Begin Source File

SOURCE="..\..\distribute-source-remove.bat"
# End Source File
# Begin Source File

SOURCE="..\..\distribute-source.bat"
# End Source File
# Begin Source File

SOURCE=..\..\distribute.bat
# End Source File
# End Group
# Begin Source File

SOURCE=..\gmake\Makefile
# End Source File
# Begin Source File

SOURCE=..\gmake\Makefile.dpvs
# End Source File
# Begin Source File

SOURCE=..\gmake\Makefile.dpvs_clean
# End Source File
# Begin Source File

SOURCE=..\gmake\Makefile.dpvs_debug
# End Source File
# Begin Source File

SOURCE=..\gmake\Makefile.inittest
# End Source File
# Begin Source File

SOURCE=..\gmake\Makefile.inittest_debug
# End Source File
# Begin Source File

SOURCE=..\gmake\Makefile.inittest_ps2
# End Source File
# Begin Source File

SOURCE=..\gmake\Makefile.inittest_ps2_debug
# End Source File
# Begin Source File

SOURCE=..\gmake\Makefile.visualizer
# End Source File
# Begin Source File

SOURCE=..\gmake\Makefile.visualizer_clean
# End Source File
# End Group
# Begin Group "WWW"

# PROP Default_Filter ""
# Begin Group "pics"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\www\pics\bottom_block.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\building1.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\building2.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\city1.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\city2.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\culling.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\dot.gif
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\left_block_01.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\left_block_02.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\left_block_03.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\left_block_04.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\left_block_05.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\left_block_06.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\left_block_07.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\left_block_08.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\left_block_09.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\left_block_10.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\left_block_11.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\left_block_12.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\main.gif
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\main.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\navivali.gif
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\occluderandme.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\right_block.gif
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\terrain.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\top_block.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\umbracover.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\umbrademo.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\umbrareflect.jpg
# End Source File
# Begin Source File

SOURCE=..\..\www\pics\umbrascout.jpg
# End Source File
# End Group
# Begin Group "download"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\www\download\dpvs_api.zip
# End Source File
# Begin Source File

SOURCE=..\..\www\download\dpvs_demos.zip
# End Source File
# Begin Source File

SOURCE=..\..\www\download\dpvs_logos.zip
# End Source File
# Begin Source File

SOURCE=..\..\www\download\dpvs_manual.zip
# End Source File
# Begin Source File

SOURCE=..\..\www\download\dpvs_online.pdf
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\www\contact.html
# End Source File
# Begin Source File

SOURCE=..\..\www\download.html
# End Source File
# Begin Source File

SOURCE=..\..\www\faq.html
# End Source File
# Begin Source File

SOURCE=..\..\www\features.html
# End Source File
# Begin Source File

SOURCE=..\..\www\index.html
# End Source File
# Begin Source File

SOURCE=..\..\www\left_block.html
# End Source File
# Begin Source File

SOURCE=..\..\www\licensing.html
# End Source File
# Begin Source File

SOURCE=..\..\www\links.html
# End Source File
# Begin Source File

SOURCE=..\..\www\news.html
# End Source File
# Begin Source File

SOURCE=..\..\www\product.html
# End Source File
# Begin Source File

SOURCE=..\..\www\umbra.css
# End Source File
# End Group
# End Target
# End Project
