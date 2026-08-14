; (Unofficial) Qt Windows Free Edition
; Copyright (C) 2004 Ralf Habacker. All rights reserved.
; Copyright (C) 2002-2004 Wolfpack Development. All rights reserved.
; Portions by TrollTech
; For conditions of distribution and use, see LICENSE.GPL.
;
; Setup script $Id: setup-bin-msvc.net.iss 1244 2005-05-31 03:50:07Z chehrlic $
;
#define RELEASE "3.3.4"
#define PATCHLEVEL "3"
;#define BASENAME "qt-win-free-msvc.net"
#define BASENAME "qt-win-free-msvc.net2003"
#define APPNAME "QT Windows Free Edition"
;#define APPVERNAME "QT Windows Free Edition 3.3.4 Patch Level 3 for Visual Studio .NET"
#define APPVERNAME "QT Windows Free Edition 3.3.4 Patch Level 3 for Visual Studio .NET2003"
#define MSVCDIR "D:\qt-3.3\vcredist"

[Setup]
AppName={# APPNAME }
AppVerName={# APPVERNAME }
LicenseFile=..\..\LICENSE.GPL
DefaultDirName={pf}\{# BASENAME}-{# RELEASE}
DefaultGroupName={# APPVERNAME }
;AppUpdatesURL=http://sourceforge.net/project/showfiles.php?group_id=38114
AppVersion={# RELEASE }
ShowTasksTreeLines=true
InfoBeforeFile=..\..\README
;DisableStartupPrompt=true
OutputBaseFilename=setup-{# BASENAME}-{# RELEASE }-{# PATCHLEVEL}
;AllowNoIcons=true
;DisableProgramGroupPage=true
;UsePreviousGroup=false
UninstallDisplayName={# APPNAME }
;InfoAfterFile=README
SolidCompression=true
UninstallDisplayIcon={app}\tools\designer\app\designer.ico
AppPublisher=The KDE-Cygwin project
AppPublisherURL=http://kde-cygwin.sf.net/qt3-win32
;AppID={381BA872-A4E7-48E3-824B-27705056EA57}
;InternalCompressLevel=ultra
;VersionInfoVersion={# RELEASE }
;VersionInfoCompany=KDE-Cygwin
;VersionInfoDescription=GPL port of the X11 Qt library to Windows
ShowLanguageDialog=yes
Compression=lzma
InternalCompressLevel=normal

[Registry]
Root: HKLM; Subkey: SYSTEM\CurrentControlSet\Control\Session Manager\Environment; ValueType: string; ValueName: QTDIR; ValueData: {app}; Flags: createvalueifdoesntexist; MinVersion: 0,4.00.1381
Root: HKLM; Subkey: SYSTEM\CurrentControlSet\Control\Session Manager\Environment; ValueType: string; ValueName: QMAKESPEC; ValueData: win32-msvc.net; Flags: createvalueifdoesntexist; MinVersion: 0,4.00.1381

[Dirs]
Name: {app}\bin
Name: {app}\doc\html
Name: {app}\examples
Name: {app}\include
Name: {app}\lib
Name: {app}\mkspecs
Name: {app}\pics
Name: {app}\plugins\accessible
Name: {app}\plugins\codecs
Name: {app}\plugins\designer
Name: {app}\plugins\imageformats
Name: {app}\plugins\sqldrivers
Name: {app}\plugins\styles
Name: {app}\src
Name: {app}\translations
Name: {app}\tutorial


[Files]
Source: {# MSVCDIR }\msvcp71.dll; DestDir: {sys}; Flags: confirmoverwrite sharedfile
Source: {# MSVCDIR }\msvcr71.dll; DestDir: {sys}; Flags: confirmoverwrite sharedfile

Source: ..\..\.qtwinconfig; DestDir: {app}
Source: ..\..\.qmake.cache; DestDir: {app}
Source: ..\..\FAQ; DestDir: {app}; DestName: FAQ.TXT
Source: ..\..\LICENSE.GPL; DestDir: {app}; DestName: LICENSE-GPL.TXT
Source: ..\..\MANIFEST; DestDir: {app}; DestName: MANIFEST.TXT
Source: ..\..\PLATFORMS; DestDir: {app}; DestName: PLATFORMS.TXT
Source: ..\..\README; DestDir: {app}; DESTNAME: README-WIN32.TXT
Source: ..\..\README-QT.TXT; DestDir: {app}; DestName: README-QT.TXT
Source: ..\..\changes-3.3.3; DestDir: {app}; DESTNAME: CHANGES-3.3.3.TXT

Source: ..\..\bin\*.exe; DestDir: {app}\bin; Excludes: link_includes.exe
Source: ..\..\bin\*.dll; DestDir: {app}\bin
Source: ..\..\bin\qt-mt3.dll; DestDir: {app}\lib
Source: ..\..\doc\html\*.*; DestDir: {app}\doc\html; Flags: recursesubdirs; Excludes: .cvsignore,CVS
Source: ..\..\examples\*.*; DestDir: {app}\examples; Flags: recursesubdirs; Excludes: .cvsignore,CVS,Makefile
Source: ..\..\include\*.h; DestDir: {app}\include; Flags: recursesubdirs; Excludes: .cvsignore,CVS
Source: ..\..\lib\*.lib; DestDir: {app}\lib; Excludes: .cvsignore,CVS
Source: ..\..\lib\*.prl; DestDir: {app}\lib
Source: ..\..\mkspecs\*.*; DestDir: {app}\mkspecs; Flags: recursesubdirs; Excludes: .cvsignore,CVS
Source: ..\..\pics\*.*; DestDir: {app}\pics
Source: ..\..\plugins\accessible\*.dll; DestDir: {app}\plugins\accessible; Flags: skipifsourcedoesntexist; Excludes: .cvsignore,CVS
Source: ..\..\plugins\codecs\*.dll; DestDir: {app}\plugins\codecs; Flags: skipifsourcedoesntexist; Excludes: .cvsignore,CVS
Source: ..\..\plugins\designer\*.dll; DestDir: {app}\plugins\designer; Flags: skipifsourcedoesntexist; Excludes: .cvsignore,CVS
Source: ..\..\plugins\imageformats\*.dll; DestDir: {app}\plugins\imageformats; Flags: skipifsourcedoesntexist; Excludes: .cvsignore,CVS
Source: ..\..\plugins\sqldrivers\*.dll; DestDir: {app}\plugins\sqldrivers; Flags: skipifsourcedoesntexist; Excludes: .cvsignore,CVS
Source: ..\..\plugins\styles\*.dll; DestDir: {app}\styles\styles; Flags: skipifsourcedoesntexist; Excludes: .cvsignore,CVS
Source: ..\..\tools\assistant\*.*; DestDir: {app}\tools\assistant; Flags: recursesubdirs; Excludes: *.pch,*.obj,tmp,.cvsignore,CVS,Makefile
Source: ..\..\tools\designer\*.*; DestDir: {app}\tools\designer; Flags: recursesubdirs; Excludes: *.pch,*.obj,tmp,.cvsignore,CVS,Makefile
Source: ..\..\tools\qembed\*.*; DestDir: {app}\tools\qembed; Flags: recursesubdirs; Excludes: *.pch,*.obj,tmp,.cvsignore,CVS,Makefile
Source: ..\..\tools\qtconfig\*.*; DestDir: {app}\tools\qtconfig; Flags: recursesubdirs; Excludes: *.pch,*.obj,tmp,.cvsignore,CVS,Makefile
Source: ..\..\tools\linguist\*.*; DestDir: {app}\tools\linguist; Flags: recursesubdirs; Excludes: *.pch,*.obj,tmp,.cvsignore,CVS,Makefile
Source: ..\..\translations\*.*; DestDir: {app}\translations; Excludes: .cvsignore,CVS
Source: ..\..\tutorial\*.*; DestDir: {app}\tutorial; Flags: recursesubdirs; Excludes: .cvsignore,CVS,Makefile

[Icons]
Name: {group}\Designer; Filename: {app}\bin\designer.exe; WorkingDir: {app}
Name: {group}\Assistant; Filename: {app}\bin\assistant.exe; WorkingDir: {app}
Name: {group}\Linguist; Filename: {app}\bin\linguist.exe; WorkingDir: {app}
Name: {group}\QtConfig; Filename: {app}\bin\qtconfig.exe; WorkingDir: {app}

Name: {group}\Release Notes; Filename: {app}\README-WIN32.TXT; WorkingDir: {app}
Name: {group}\ChangeLog; Filename: {app}\CHANGES-3.3.3.TXT; WorkingDir: {app}
Name: {group}\FAQ; Filename: {app}\FAQ.TXT; WorkingDir: {app}
;Name: {group}\Q Public License (QPL); Filename: "{app}\LICENSE-QPL.TXT"; WorkingDir: "{app}";
Name: {group}\General Public License (GPL); Filename: {app}\LICENSE-GPL.TXT; WorkingDir: {app}
Name: {group}\KDE-Cygwin project; Filename: http://kde-cygwin.sourceforge.net/qt3-win32
Name: {group}\Trolltech - Qt C++ application framework; Filename: http://www.trolltech.com
Name: {group}\Uninstall {# APPNAME}; Filename: {uninstallexe}
[Run]
Filename: {app}\bin\assistant.exe; Parameters: -addcontentfile {app}\doc\html\qt.xml; WorkingDir: {app}\bin; StatusMsg: Add qt documentation
Filename: {app}\bin\assistant.exe; Parameters: -addcontentfile {app}\doc\html\assistant.xml; WorkingDir: {app}\bin; StatusMsg: Add assistant documentation
Filename: {app}\bin\assistant.exe; Parameters: -addcontentfile {app}\doc\html\designer.xml; WorkingDir: {app}\bin; StatusMsg: Add designer documentation
Filename: {app}\bin\assistant.exe; Parameters: -addcontentfile {app}\doc\html\linguist.xml; WorkingDir: {app}\bin; StatusMsg: Add linguist documentation
Filename: {app}\bin\assistant.exe; Parameters: -addcontentfile {app}\doc\html\qmake.xml; WorkingDir: {app}\bin; StatusMsg: Add qmake documentation
[Languages]
Name: English; MessagesFile: compiler:Default.isl
Name: German; MessagesFile: compiler:Languages\German.isl
