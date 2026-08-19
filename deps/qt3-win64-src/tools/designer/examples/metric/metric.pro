TEMPLATE = app
LANGUAGE = C++

SOURCES	+= main.cpp
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= metric.ui
CONFIG	+= qt warn_on release
DBFILE	= metric.db
