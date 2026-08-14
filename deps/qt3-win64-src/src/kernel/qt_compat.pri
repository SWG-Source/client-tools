# Qt compatibility

# scratch pad for internal development
# hack these for your build like
# internal {
#     CONFIG += blah
# }

##########################################################

# mac hac fu
!embedded:!x11:mac {
     #never
     CONFIG -= nas x11 x11sm
     #CONFIG += sqlcrap
     sql:sqlcrap {
	sql-drivers += postgres
	INCLUDEPATH+=/Users/sam/postgresql-7.0.2/src/include \
		    /Users/sam/postgresql-7.0.2/src/interfaces/libpq
	LIBS += -L/Users/sam/postgresql-7.0.2/src/interfaces/libpq 
     }
} 

attic {
	SOURCES += attic/qttableview.cpp
	HEADERS += attic/qttableview.h
}
