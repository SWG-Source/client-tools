# Additional Qt project file for qtmain lib on Windows
TEMPLATE = lib
TARGET	 = qtmain
VERSION	 = 3.1.0
DESTDIR	 = $$QMAKE_LIBDIR_QT

CONFIG	+= qt staticlib warn_on release
CONFIG	-= dll

win32 {
	SOURCES		= kernel/qtmain_win.cpp
	CONFIG		+= png zlib
	CONFIG		-= jpeg
	INCLUDEPATH	+= tmp
}
win32-borland:INCLUDEPATH += kernel

!win32-*:!wince-*:error("${QMAKE_FILE} is intended only for Windows!")
