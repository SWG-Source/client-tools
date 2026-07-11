TEMPLATE = app
LANGUAGE = C++
CONFIG	+= qt warn_on


SOURCES	+= finddialog.cpp \
	main.cpp \
	msgedit.cpp \
	phrase.cpp \
	phrasebookbox.cpp \
	phraselv.cpp \
	printout.cpp \
	simtexth.cpp \
	trwindow.cpp \
	listviews.cpp \
	../shared/metatranslator.cpp
HEADERS	+= finddialog.h \
	msgedit.h \
	phrase.h \
	phrasebookbox.h \
	phraselv.h \
	printout.h \
	trwindow.h \
	listviews.h \
	../shared/metatranslator.h


TRANSLATIONS	= linguist_de.ts \
		  linguist_fr.ts

DEFINES	+= QT_INTERNAL_XML
include( ../../../src/qt_professional.pri )

DESTDIR		= ../../../bin
TARGET		= linguist


LIBS	+= -L$$QT_BUILD_TREE/lib -lqassistantclient

win32:RC_FILE	= linguist.rc

mac {
    staticlib:CONFIG -= global_init_link_order #yuck
    RC_FILE = linguist.icns
}

PROJECTNAME	= Qt Linguist

target.path=$$bins.path
INSTALLS	+= target

linguisttranslations.files = *.qm
linguisttranslations.path = $$translations.path
INSTALLS += linguisttranslations

phrasebooks.path=$$data.path/phrasebooks
phrasebooks.files = ../phrasebooks/*
INSTALLS += phrasebooks
FORMS	= about.ui \
	statistics.ui
IMAGES	= images/accelerator.png \
	images/book.png \
	images/s_check_danger.png \
	images/s_check_obs.png \
	images/s_check_off.png \
	images/s_check_on.png \
	images/d_s_check_danger.png \
	images/d_s_check_obs.png \
	images/d_s_check_off.png \
	images/d_s_check_on.png \
	images/d_accelerator.png \
	images/d_book.png \
	images/d_doneandnext.png \
	images/d_editcopy.png \
	images/d_editcut.png \
	images/d_editpaste.png \
	images/d_fileopen.png \
	images/d_filesave.png \
	images/d_next.png \
	images/d_nextunfinished.png \
	images/d_prev.png \
	images/d_prevunfinished.png \
	images/d_phrase.png \
	images/d_print.png \
	images/d_punctuation.png \
	images/d_redo.png \
	images/d_searchfind.png \
	images/d_undo.png \
	images/doneandnext.png \
	images/editcopy.png \
	images/editcut.png \
	images/editpaste.png \
	images/punctuation.png \
	images/fileopen.png \
	images/filesave.png \
	images/appicon.png \
	images/next.png \
	images/nextunfinished.png \
	images/pagecurl.png \
	images/phrase.png \
	images/prev.png \
	images/prevunfinished.png \
	images/print.png \
	images/redo.png \
	images/searchfind.png \
	images/splash.png \
	images/undo.png \
	images/whatsthis.xpm
INCLUDEPATH	+= ../shared
