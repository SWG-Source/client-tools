# Qt xml module

xml {
	HEADERS += $$XML_H/qxml.h $$XML_H/qdom.h $$XML_CPP/qsvgdevice_p.h
	SOURCES += $$XML_CPP/qxml.cpp $$XML_CPP/qdom.cpp $$XML_CPP/qsvgdevice.cpp
	win32-borland {
		QMAKE_CFLAGS_WARN_ON	+= -w-use
		QMAKE_CXXFLAGS_WARN_ON	+= -w-use
	}
}
