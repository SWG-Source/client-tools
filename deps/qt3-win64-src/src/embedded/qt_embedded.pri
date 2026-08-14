# Qt/Embedded Drivers

embedded {
	EMBEDDED_P = embedded

	HEADERS += $$EMBEDDED_P/qgfxdriverinterface_p.h \
		    $$EMBEDDED_H/qgfxdriverplugin_qws.h \
		    $$EMBEDDED_H/qgfxdriverfactory_qws.h \
		    $$EMBEDDED_H/qkbd_qws.h \
		    $$EMBEDDED_P/qkbddriverinterface_p.h \
		    $$EMBEDDED_H/qkbddriverplugin_qws.h \
		    $$EMBEDDED_H/qkbddriverfactory_qws.h \
		    $$EMBEDDED_H/qmouse_qws.h \
		    $$EMBEDDED_P/qmousedriverinterface_p.h \
		    $$EMBEDDED_H/qmousedriverplugin_qws.h \
		    $$EMBEDDED_H/qmousedriverfactory_qws.h

	SOURCES += $$EMBEDDED_CPP/qgfxdriverplugin_qws.cpp \
		    $$EMBEDDED_CPP/qgfxdriverfactory_qws.cpp \
		    $$EMBEDDED_CPP/qkbd_qws.cpp \
		    $$EMBEDDED_CPP/qkbddriverplugin_qws.cpp \
		    $$EMBEDDED_CPP/qkbddriverfactory_qws.cpp \
		    $$EMBEDDED_CPP/qmouse_qws.cpp \
		    $$EMBEDDED_CPP/qmousedriverplugin_qws.cpp \
		    $$EMBEDDED_CPP/qmousedriverfactory_qws.cpp

#
# Graphics drivers
#
        linux-* {
	        HEADERS += $$EMBEDDED_H/qgfxlinuxfb_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxlinuxfb_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_LINUXFB

	contains( gfx-drivers, qvfb ) {
		HEADERS += $$EMBEDDED_H/qgfxvfb_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxvfb_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_VFB

	contains( gfx-drivers, vnc ) {
		HEADERS += $$EMBEDDED_H/qgfxvnc_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxvnc_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_VNC

	!contains( DEFINES, QT_NO_QWS_LINUXFB):contains( gfx-drivers, vga16 ) {
		HEADERS += $$EMBEDDED_H/qgfxvga16_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxvga16_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_VGA16

	contains( gfx-drivers, transformed ) {
		HEADERS += $$EMBEDDED_H/qgfxtransformed_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxtransformed_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_TRANSFORMED

	contains( gfx-drivers, snap ) {
        exists( $(SCITECH)/include/snap/graphics.h) {
    		HEADERS += $$EMBEDDED_H/qgfxsnap_qws.h
    		SOURCES += $$EMBEDDED_CPP/qgfxsnap_qws.cpp
            INCLUDEPATH += $(SCITECH)/include
            debug:LIBS  += -L$(SCITECH)/lib/debug/linux/gcc/x86/so -lpm
            else:LIBS   += -L$(SCITECH)/lib/release/linux/gcc/x86/so -lpm
        }
        else {
            message("SciTech SNAP SDK is not properly set up! Please make sure the SCITECH")
            message("environment variable is pointing to the SciTech SNAP SDK.")
            error("Please fix and re-build the makefiles.")
        }
	}
	else:DEFINES += QT_NO_QWS_SNAP

	contains( gfx-drivers, mach64 ) {
		HEADERS += $$EMBEDDED_H/qgfxmach64_qws.h \
			   $$EMBEDDED_H/qgfxmach64defs_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxmach64_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_MACH64

	contains( gfx-drivers, voodoo ) {
		HEADERS += $$EMBEDDED_H/qgfxvoodoo_qws.h \
			   $$EMBEDDED_H/qgfxvoodoodefs_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxvoodoo_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_VOODOO3

	contains( gfx-drivers, matrox ) {
		HEADERS += $$EMBEDDED_H/qgfxmatrox_qws.h \
			   $$EMBEDDED_H/qgfxmatroxdefs_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxmatrox_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_MATROX

	contains( gfx-drivers, shadowfb ) {
		HEADERS += $$EMBEDDED_H/qgfxshadow_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxshadow_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_SHADOWFB

	contains( gfx-drivers, repeater ) {
		HEADERS += $$EMBEDDED_H/qgfxrepeater_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxrepeater_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_REPEATER

#
# Keyboard drivers
#

	contains( kbd-drivers, sl5000 ) {
		HEADERS +=$$EMBEDDED_H/qkbdsl5000_qws.h
		SOURCES +=$$EMBEDDED_CPP/qkbdsl5000_qws.cpp
		!contains( kbd-drivers, tty ) {
		    kbd-drivers += tty
		}
	}
	else:DEFINES += QT_NO_QWS_KBD_SL5000

	contains( kbd-drivers, tty ) {
		HEADERS +=$$EMBEDDED_H/qkbdtty_qws.h
		SOURCES +=$$EMBEDDED_CPP/qkbdtty_qws.cpp
		!contains( kbd-drivers, pc101 ) {
		    kbd-drivers += pc101
		}
	}
	else:DEFINES += QT_NO_QWS_KBD_TTY

	contains( kbd-drivers, usb ) {
		HEADERS +=$$EMBEDDED_H/qkbdusb_qws.h
		SOURCES +=$$EMBEDDED_CPP/qkbdusb_qws.cpp
		!contains( kbd-drivers, pc101 ) {
		    kbd-drivers += pc101
		}
	}
	else:DEFINES += QT_NO_QWS_KBD_USB

	contains( kbd-drivers, pc101 ) {
		HEADERS +=$$EMBEDDED_H/qkbdpc101_qws.h
		SOURCES +=$$EMBEDDED_CPP/qkbdpc101_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_KBD_PC101

	contains( kbd-drivers, yopy ) {
		HEADERS +=$$EMBEDDED_H/qkbdyopy_qws.h
		SOURCES +=$$EMBEDDED_CPP/qkbdyopy_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_KBD_YOPY

	contains( kbd-drivers, vr41xx ) {
		HEADERS +=$$EMBEDDED_H/qkbdvr41xx_qws.h
		SOURCES +=$$EMBEDDED_CPP/qkbdvr41xx_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_KBD_VR41

#
# Mouse drivers
#

	contains( mouse-drivers, pc ) {
		HEADERS +=$$EMBEDDED_H/qmousepc_qws.h
		SOURCES +=$$EMBEDDED_CPP/qmousepc_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_MOUSE_PC

	contains( mouse-drivers, bus ) {
		HEADERS +=$$EMBEDDED_H/qmousebus_qws.h
		SOURCES +=$$EMBEDDED_CPP/qmousebus_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_MOUSE_BUS

	contains( mouse-drivers, linuxtp ) {
		HEADERS +=$$EMBEDDED_H/qmouselinuxtp_qws.h
		SOURCES +=$$EMBEDDED_CPP/qmouselinuxtp_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_MOUSE_LINUXTP

	contains( mouse-drivers, vr41xx ) {
		HEADERS +=$$EMBEDDED_H/qmousevr41xx_qws.h
		SOURCES +=$$EMBEDDED_CPP/qmousevr41xx_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_MOUSE_VR41

	contains( mouse-drivers, yopy ) {
		HEADERS +=$$EMBEDDED_H/qmouseyopy_qws.h
		SOURCES +=$$EMBEDDED_CPP/qmouseyopy_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_MOUSE_YOPY
}

