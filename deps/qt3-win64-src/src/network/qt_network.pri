# Qt network module

network {
	HEADERS += $$NETWORK_H/qdns.h \
		    $$NETWORK_H/qftp.h \
		    $$NETWORK_H/qhttp.h \
		    $$NETWORK_H/qhostaddress.h \
		    $$NETWORK_H/qnetwork.h \
		    $$NETWORK_H/qserversocket.h \
		    $$NETWORK_H/qsocket.h \
		    $$NETWORK_H/qsocketdevice.h
	NETWORK_SOURCES	= $$NETWORK_CPP/qdns.cpp \
		    $$NETWORK_CPP/qftp.cpp \
		    $$NETWORK_CPP/qhttp.cpp \
		    $$NETWORK_CPP/qhostaddress.cpp \
		    $$NETWORK_CPP/qnetwork.cpp \
		    $$NETWORK_CPP/qserversocket.cpp \
		    $$NETWORK_CPP/qsocket.cpp \
		    $$NETWORK_CPP/qsocketdevice.cpp
	unix:NETWORK_SOURCES += $$NETWORK_CPP/qsocketdevice_unix.cpp
	win32:NETWORK_SOURCES += $$NETWORK_CPP/qsocketdevice_win.cpp
	SOURCES    += $$NETWORK_SOURCES
}
