TEMPLATE = app
TARGET	 = richedit
LANGUAGE = C++

CONFIG	+= qt warn_on release
SOURCES	+= main.cpp
IMAGEFILE	= images.cpp
FORMS	= richedit.ui
IMAGES	= images/editcopy.png images/editcut.png images/editpaste.png images/filenew.png images/fileopen.png images/filesave.png images/redo.png images/textbold.png images/textcenter.png images/textitalic.png images/textleft.png images/textright.png images/textunder.png images/undo.png
DBFILE	= richedit.db
