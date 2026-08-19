TEMPLATE 	= lib
CONFIG		+= qt warn_on plugin

SOURCES		+= cppeditor.cpp syntaxhighliter_cpp.cpp cppcompletion.cpp editorinterfaceimpl.cpp languageinterfaceimpl.cpp common.cpp preferenceinterfaceimpl.cpp yyreg.cpp cppbrowser.cpp projectsettingsinterfaceimpl.cpp sourcetemplateinterfaceimpl.cpp
HEADERS		+= cppeditor.h syntaxhighliter_cpp.h cppcompletion.h editorinterfaceimpl.h languageinterfaceimpl.h preferenceinterfaceimpl.h yyreg.h cppbrowser.h projectsettingsinterfaceimpl.h sourcetemplateinterfaceimpl.h
FORMS		= projectsettings.ui mainfilesettings.ui

TARGET		= cppeditor
DESTDIR		= ../../../../plugins/designer
VERSION		= 1.0.0

INCLUDEPATH	+= ../../interfaces ../../editor $$QT_BUILD_TREE/tools/designer/editor

hpux-* {
	LIBS += $$QT_BUILD_TREE/lib/libeditor.a
} else {
	LIBS    += -L$$QT_BUILD_TREE/lib -leditor
}

target.path += $$plugins.path/designer
INSTALLS 	+= target
