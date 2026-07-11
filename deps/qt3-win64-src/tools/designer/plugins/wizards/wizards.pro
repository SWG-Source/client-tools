TEMPLATE	= lib
CONFIG		+= qt warn_on release plugin

SOURCES		+= main.cpp
FORMS		= sqlformwizard.ui mainwindowwizard.ui

sql {
	HEADERS		+= sqlformwizardimpl.h
	SOURCES		+= sqlformwizardimpl.cpp
	}
DESTDIR		= ../../../../plugins/designer

PROJECTNAME	= Wizards
IMAGES		= images/qtwizards_menu_1.png images/qtwizards_menu_2.png images/qtwizards_menu_3.png images/qtwizards_table_1.png images/qtwizards_table_2.png images/qtwizards_table_3.png images/qtwizards_table_4.png images/re-sort.png images/small/editcopy.png images/small/editcut.png images/small/editpaste.png images/small/filenew.png images/small/fileopen.png images/small/filesave.png images/small/print.png images/small/redo.png images/small/searchfind.png images/small/undo.png
INCLUDEPATH	+= ../../interfaces
DBFILE		= wizards.db
LANGUAGE	= C++


target.path += $$plugins.path/designer
INSTALLS 	+= target
