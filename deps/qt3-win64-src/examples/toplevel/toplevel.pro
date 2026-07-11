TEMPLATE = app
LANGUAGE = C++

CONFIG	+= qt warn_on release
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}

REQUIRES = full-config

SOURCES	+= main.cpp
FORMS	 = options.ui
