#############################################
#
# Example for using Precompiled Headers
#
#############################################
TEMPLATE  = app
LANGUAGE  = C++
CONFIG	 += console precompile_header

# Use Precompiled headers (PCH)
PRECOMPILED_HEADER  = stable.h

HEADERS	 += stable.h \
            myobject.h
SOURCES	 += main.cpp \
            myobject.cpp \
            util.cpp
FORMS     = mydialog.ui

