# Qt codecs module

!bigcodecs:DEFINES += QT_NO_BIG_CODECS
tools {
	CODECS_P		= codecs
	HEADERS += \
#$$CODECS_H/qasmocodec.h \
		$$CODECS_H/qbig5codec.h \
		  $$CODECS_H/qeucjpcodec.h \
		  $$CODECS_H/qeuckrcodec.h \
		  $$CODECS_P/qisciicodec_p.h \
		  $$CODECS_H/qgb18030codec.h \
		  $$CODECS_H/qjiscodec.h \
		  $$CODECS_H/qjpunicode.h \
		  $$CODECS_H/qrtlcodec.h \
		  $$CODECS_H/qsjiscodec.h \
		  $$CODECS_H/qtextcodec.h \
		  $$CODECS_H/qtsciicodec.h \
		  $$CODECS_H/qutfcodec.h \
		  $$CODECS_P/qtextcodecinterface_p.h \
		  $$CODECS_H/qtextcodecfactory.h \
		  $$CODECS_H/qtextcodecplugin.h

	SOURCES += \
#$$CODECS_CPP/qasmocodec.cpp \
		$$CODECS_CPP/qbig5codec.cpp \
		  $$CODECS_CPP/qeucjpcodec.cpp \
		  $$CODECS_CPP/qeuckrcodec.cpp \
		  $$CODECS_CPP/qisciicodec.cpp \
		  $$CODECS_CPP/qgb18030codec.cpp \
		  $$CODECS_CPP/qjiscodec.cpp \
		  $$CODECS_CPP/qjpunicode.cpp \
		  $$CODECS_CPP/qrtlcodec.cpp \
		  $$CODECS_CPP/qsjiscodec.cpp \
		  $$CODECS_CPP/qtextcodec.cpp \
		  $$CODECS_CPP/qtsciicodec.cpp \
		  $$CODECS_CPP/qutfcodec.cpp \
		  $$CODECS_CPP/qtextcodecfactory.cpp \
		  $$CODECS_CPP/qtextcodecplugin.cpp

	x11:SOURCES += $$CODECS_CPP/qfontcncodec.cpp  \
	               $$CODECS_CPP/qfonthkcodec.cpp  \
	               $$CODECS_CPP/qfontjpcodec.cpp  \
		       $$CODECS_CPP/qfontkrcodec.cpp  \
		       $$CODECS_CPP/qfontlaocodec.cpp \
		       $$CODECS_CPP/qfonttwcodec.cpp
	x11:HEADERS += $$CODECS_P/qfontcodecs_p.h
}
