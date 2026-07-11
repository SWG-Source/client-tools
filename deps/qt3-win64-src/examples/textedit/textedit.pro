TEMPLATE	= app
TARGET		= textedit

CONFIG		+= qt warn_on release

REQUIRES	= full-config nocrosscompiler

HEADERS		= textedit.h
SOURCES		= textedit.cpp \
		  main.cpp
IMAGES 		= editcopy.xpm editcut.xpm editpaste.xpm editredo.xpm editundo.xpm filenew.xpm fileopen.xpm fileprint.xpm filesave.xpm textbold.xpm textcenter.xpm textitalic.xpm textjustify.xpm textleft.xpm textright.xpm textunder.xpm
