# Qt tools module

tools {
	TOOLS_P		= tools
	HEADERS +=  $$TOOLS_H/qmemarray.h \
		  $$TOOLS_H/qasciicache.h \
		  $$TOOLS_H/qasciidict.h \
		  $$TOOLS_H/qbitarray.h \
		  $$TOOLS_H/qbuffer.h \
		  $$TOOLS_H/qcache.h \
		  $$TOOLS_H/qcleanuphandler.h \
		  $$TOOLS_P/qcomponentfactory_p.h \
		  $$TOOLS_P/qcomlibrary_p.h \
		  $$TOOLS_H/qcstring.h \
		  $$TOOLS_H/qdatastream.h \
		  $$TOOLS_H/qdatetime.h \
		  $$TOOLS_H/qdeepcopy.h \
		  $$TOOLS_H/qdict.h \
		  $$TOOLS_H/qdir.h \
		  $$TOOLS_P/qdir_p.h \
		  $$TOOLS_H/qfile.h \
		  $$TOOLS_P/qfiledefs_p.h \
		  $$TOOLS_H/qfileinfo.h \
		  $$TOOLS_H/qgarray.h \
		  $$TOOLS_H/qgcache.h \
		  $$TOOLS_H/qgdict.h \
		  $$TOOLS_H/qgeneric.h \
		  $$TOOLS_H/qglist.h \
		  $$TOOLS_H/qglobal.h \
		  $$TOOLS_P/qgpluginmanager_p.h \
		  $$TOOLS_H/qgvector.h \
		  $$TOOLS_H/qintcache.h \
		  $$TOOLS_H/qintdict.h \
		  $$TOOLS_H/qiodevice.h \
		  $$TOOLS_H/qlibrary.h \
		  $$TOOLS_P/qlibrary_p.h \
		  $$TOOLS_H/qlocale.h \
		  $$TOOLS_P/qlocale_p.h \
		  $$TOOLS_H/qptrlist.h \
		  $$TOOLS_H/qmap.h \
		  $$TOOLS_H/qmutex.h \
		  $$TOOLS_P/qmutex_p.h \
		  $$TOOLS_P/qmutexpool_p.h \
		  $$TOOLS_P/qpluginmanager_p.h \
		  $$TOOLS_H/qptrcollection.h \
		  $$TOOLS_H/qptrdict.h \
		  $$TOOLS_H/qptrqueue.h \
		  $$TOOLS_H/qregexp.h \
		  $$TOOLS_H/qsemaphore.h \
		  $$TOOLS_H/qsettings.h \
		  $$TOOLS_P/qsettings_p.h \
		  $$TOOLS_H/qshared.h \
		  $$TOOLS_H/qsortedlist.h \
		  $$TOOLS_H/qptrstack.h \
		  $$TOOLS_H/qstring.h \
		  $$TOOLS_H/qstringlist.h \
		  $$TOOLS_H/qstrlist.h \
		  $$TOOLS_H/qstrvec.h \
		  $$TOOLS_H/qtextstream.h \
		  $$TOOLS_P/qthreadinstance_p.h \
		  $$TOOLS_H/qthreadstorage.h\
		  $$TOOLS_P/qunicodetables_p.h \
		  $$TOOLS_H/qptrvector.h \
	          $$TOOLS_H/qvaluelist.h \
		  $$TOOLS_H/qvaluestack.h \
		  $$TOOLS_H/qvaluevector.h \
		  $$TOOLS_H/qwaitcondition.h \
		  $$TOOLS_P/qcom_p.h \
		  $$TOOLS_P/qucom_p.h \
		  $$TOOLS_H/quuid.h

	win32:!unix:SOURCES += $$TOOLS_CPP/qdir_win.cpp \
	 	  $$TOOLS_CPP/qfile_win.cpp \
		  $$TOOLS_CPP/qfileinfo_win.cpp \
		  $$TOOLS_CPP/qlibrary_win.cpp \
		  $$TOOLS_CPP/qsettings_win.cpp \
		  $$TOOLS_CPP/qmutex_win.cpp \
		  $$TOOLS_CPP/qwaitcondition_win.cpp \
		  $$TOOLS_CPP/qthreadstorage_win.cpp \
		  $$TOOLS_CPP/qcriticalsection_p.cpp

	win32:unix:SOURCES += $$TOOLS_CPP/qsettings_win.cpp \
			  $$TOOLS_CPP/qcriticalsection_p.cpp

	win32-borland:SOURCES += $$TOOLS_CPP/qwinexport.cpp

	wince-* {
		SOURCES -= $$TOOLS_CPP/qdir_win.cpp \
			   $$TOOLS_CPP/qfile_win.cpp \
			   $$TOOLS_CPP/qfileinfo_win.cpp
		SOURCES += $$TOOLS_CPP/qdir_wce.cpp \
			   $$TOOLS_CPP/qfile_wce.cpp \
			   $$TOOLS_CPP/qfileinfo_wce.cpp
	}

        offmac:SOURCES += $$TOOLS_CPP/qdir_mac.cpp \
		  $$TOOLS_CPP/qfile_mac.cpp \
		  $$TOOLS_CPP/qfileinfo_mac.cpp
	else:unix:SOURCES += $$TOOLS_CPP/qdir_unix.cpp \
		  $$TOOLS_CPP/qfile_unix.cpp \
		  $$TOOLS_CPP/qfileinfo_unix.cpp \
		  $$TOOLS_CPP/qmutex_unix.cpp \
		  $$TOOLS_CPP/qthreadstorage_unix.cpp \
		  $$TOOLS_CPP/qwaitcondition_unix.cpp

        mac:!x11:!embedded:SOURCES += $$TOOLS_CPP/qsettings_mac.cpp
	mac {
		SOURCES+=3rdparty/dlcompat/dlfcn.c
		INCLUDEPATH+=3rdparty/dlcompat
	    }
	unix:SOURCES += $$TOOLS_CPP/qlibrary_unix.cpp

	SOURCES += $$TOOLS_CPP/qbitarray.cpp \
		  $$TOOLS_CPP/qbuffer.cpp \
		  $$TOOLS_CPP/qcomponentfactory.cpp \
		  $$TOOLS_CPP/qcomlibrary.cpp \
		  $$TOOLS_CPP/qcstring.cpp \
		  $$TOOLS_CPP/qdatastream.cpp \
		  $$TOOLS_CPP/qdatetime.cpp \
		  $$TOOLS_CPP/qdeepcopy.cpp \
		  $$TOOLS_CPP/qdir.cpp \
		  $$TOOLS_CPP/qfile.cpp \
		  $$TOOLS_CPP/qfileinfo.cpp \
		  $$TOOLS_CPP/qgarray.cpp \
		  $$TOOLS_CPP/qgcache.cpp \
		  $$TOOLS_CPP/qgdict.cpp \
		  $$TOOLS_CPP/qglist.cpp \
		  $$TOOLS_CPP/qglobal.cpp \
		  $$TOOLS_CPP/qgpluginmanager.cpp \
		  $$TOOLS_CPP/qgvector.cpp \
		  $$TOOLS_CPP/qiodevice.cpp \
		  $$TOOLS_CPP/qlibrary.cpp \
		  $$TOOLS_CPP/qlocale.cpp \
		  $$TOOLS_CPP/qmap.cpp \
		  $$TOOLS_CPP/qmutexpool.cpp \
		  $$TOOLS_CPP/qptrcollection.cpp \
		  $$TOOLS_CPP/qregexp.cpp \
		  $$TOOLS_CPP/qstring.cpp \
		  $$TOOLS_CPP/qsemaphore.cpp \
		  $$TOOLS_CPP/qsettings.cpp \
		  $$TOOLS_CPP/qstringlist.cpp \
		  $$TOOLS_CPP/qtextstream.cpp \
		  $$TOOLS_CPP/qunicodetables.cpp \
		  $$TOOLS_CPP/qucom.cpp \
		  $$TOOLS_CPP/quuid.cpp

        irix-cc* {
                CXXFLAGS_PRELINK = $$QMAKE_CXXFLAGS
                CXXFLAGS_PRELINK -= -O2
                QMAKE_PRE_LINK          = $(CXX) -c $$CXXFLAGS_PRELINK -O1 $(INCPATH) -o $(OBJECTS_DIR)/qlocale.o tools/qlocale.cpp
        }
}

