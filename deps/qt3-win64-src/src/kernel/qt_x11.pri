unix {
	!xinerama:DEFINES += QT_NO_XINERAMA
	!xshape:DEFINES += QT_NO_SHAPE
	!xcursor:DEFINES += QT_NO_XCURSOR
	!xrandr:DEFINES += QT_NO_XRANDR
	!xrender:DEFINES += QT_NO_XRENDER
	!xftfreetype:DEFINES += QT_NO_XFTFREETYPE
	!xkb:DEFINES += QT_NO_XKB
	xft2header:DEFINES+=QT_USE_XFT2_HEADER

	SOURCES += $$KERNEL_CPP/qtaddons_x11.cpp
	PRECOMPILED_HEADER = kernel/qt_pch.h
}

nas {
	DEFINES     += QT_NAS_SUPPORT
	LIBS	+= -laudio -lXt
}

!x11sm:DEFINES += QT_NO_SM_SUPPORT
