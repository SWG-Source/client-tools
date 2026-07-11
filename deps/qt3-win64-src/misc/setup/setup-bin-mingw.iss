; (Unofficial) Qt Windows Free Edition
; Copyright (C) 2002-2004 Wolfpack Development. All rights reserved.
; Copyright (C) 2004-2005 Ralf Habacker. All rights reserved.
; Portions by TrollTech
; For conditions of distribution and use, see LICENSE.GPL.
;
; Setup script $Id: setup-bin-mingw.iss 1245 2005-06-11 09:16:19Z chehrlic $
;
#define RELEASE "3.3.4"
#define PATCHLEVEL "3"
#define BASENAME "qt-win-free-mingw"
#define APPNAME "QT Windows Free Edition"
#define APPVERNAME "QT Windows Free Edition 3.3.4 Patch Level 3 for MingW"
#define QMAKESPEC "win32-g++"
#define MINGWDIR "C:\Programme\MinGW"

[Files]
Source: {# MINGWDIR}\bin\mingwm10.dll; DestDir: {app}\bin
Source: ..\..\lib\libqt*.a; DestDir: {app}\lib

#include "setup-bin-main.iss"
