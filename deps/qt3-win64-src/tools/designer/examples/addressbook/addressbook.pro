TEMPLATE = app
LANGUAGE = C++

CONFIG	+= qt warn_on release
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}

SOURCES	+= main.cpp
FORMS	= addressbook.ui addressdetails.ui search.ui
IMAGES	= images/filenew.png images/fileopen.png images/filesave.png images/searchfind.png images/editcut.png
DBFILE	= addressbook.db
