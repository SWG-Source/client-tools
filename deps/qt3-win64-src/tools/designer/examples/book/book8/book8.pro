TEMPLATE = app
LANGUAGE = C++

SOURCES	+= main.cpp ../connection.cpp
FORMS	= book.ui editbook.ui
CONFIG	+= qt warn_on release
DBFILE	= book.db
