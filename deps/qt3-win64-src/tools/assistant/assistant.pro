TEMPLATE = app
LANGUAGE = C++
TARGET	 = assistant

CONFIG	+= qt warn_off

PROJECTNAME	= Assistant
DESTDIR		= ../../bin

SOURCES	+= main.cpp \
	helpwindow.cpp \
	topicchooserimpl.cpp \
	docuparser.cpp \
	helpdialogimpl.cpp \
	settingsdialogimpl.cpp \
	index.cpp \
        profile.cpp \
        config.cpp 

HEADERS	+= helpwindow.h \
	topicchooserimpl.h \
	docuparser.h \
	helpdialogimpl.h \
	settingsdialogimpl.h \
	index.h \
        profile.h \
        config.h

#DEFINES +=  QT_PALMTOPCENTER_DOCS
DEFINES	+= QT_INTERNAL_NETWORK QT_INTERNAL_XML
include( ../../src/qt_professional.pri )

win32:RC_FILE = assistant.rc
mac:RC_FILE = assistant.icns

target.path = $$bins.path
INSTALLS += target

assistanttranslations.files = *.qm
assistanttranslations.path = $$translations.path
INSTALLS += assistanttranslations

TRANSLATIONS	= assistant_de.ts \
		  assistant_fr.ts

unix:!zlib:LIBS	+= -lz

FORMS	= mainwindow.ui \
	topicchooser.ui \
	finddialog.ui \
	helpdialog.ui \
	settingsdialog.ui \
	tabbedbrowser.ui
IMAGES	= images/editcopy.png \
	images/find.png \
	images/home.png \
	images/next.png \
	images/previous.png \
	images/print.png \
	images/whatsthis.xpm \
	images/book.png \
	images/designer.png \
	images/assistant.png \
	images/linguist.png \
	images/qt.png \
	images/zoomin.png \
	images/zoomout.png \
	images/splash.png \
	images/appicon.png \
	images/addtab.png \
	images/closetab.png \
	images/d_closetab.png
