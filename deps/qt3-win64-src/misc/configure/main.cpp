/****************************************************************************
**
** Copyright (C) 2003-2004 Wolfpack.  All rights reserved.
** Copyright (C) 2004 Ralf Habacker
** Copyright (C) 2004 Andreas Hausladen
** Copyright (C) 2004,2005 Christian Ehrlicher
**
** This file is part of configure.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
**********************************************************************/

#include <windows.h>        // SearchPath()
#include <qnamespace.h>
#include <qregexp.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <qvaluelist.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

// for Borland, main is defined to qMain which breaks configure
#undef main

#define DONT_SUPPORT_WINDOWSXP_STYLE 1

FILE* out = stdout;

void usage( void );

struct Options
{
    QString install_prefix;
    QString QT_INSTALL_DOCS;
    QString QT_INSTALL_HEADERS;
    QString QT_INSTALL_LIBS;
    QString QT_INSTALL_BINS;
    QString QT_INSTALL_PLUGINS;
    QString QT_INSTALL_DATA;
    QString QT_INSTALL_TRANSLATIONS;
    QString QT_INSTALL_SYSCONF;

    QStringList QConfig;
    QStringList QCFlags;
    QStringList QModules;
    QStringList QNoModules;
    QStringList QStylesAvailable;
    QStringList QStyles;
    QStringList QStylesPlugin;
    QStringList QStylesNo;
    QStringList QImageFmtsAvailable;
    QStringList QImageFmts;
    QStringList QImageFmtsPlugin;
    QStringList QImageFmtsNo;
    QStringList sqlDriverAvailable;
    QStringList sqlDriver;
    QStringList sqlDriverPlugin;
    QStringList sqlDriverNo;
    QString qmakespec;
    bool build;
    bool verbose;
    bool fast;
    bool rebuild;
    bool makeFiles;
    bool dryrun;
    bool createDSPFiles;
    char *logfile;
}
options;

void my_debug( const QString &s )
{
    if ( options.verbose ) {
        printf( "[debug]: %s\n", s.latin1() );
    }
}

void my_system( const QString &s )
{
    my_debug ( s );
    if ( !options.dryrun )
        system( s );
}

QString getMake()
{
    QString makespec = options.qmakespec;
    if ( makespec.startsWith( "win32-msvc" ) )
        return "nmake /NOLOGO";
    else if ( makespec == "win32-g++" )
        return "mingw32-make";
    else if ( makespec == "win32-borland" ) {
        char * fp, *buf = "";
        int len;
        /* use bmake if available, otherwise use make */
        len = SearchPathA ( NULL, "bmake.exe", NULL, 0, buf, &fp );
        if ( len )
            return "bmake";
    }
    return "make";
}

/* returns the correct Makefile* for building qmake... */
QString getMakefile()
{
    QString makespec = options.qmakespec;
    if ( makespec.startsWith( "win32-msvc" ) )
        return "Makefile.win32-msvc";
    if ( makespec == "win32-g++" )
        return "Makefile.win32-g++";
    else if ( makespec == "win32-borland" )
        return "Makefile.win32-borland";
    return "Makefile";
}

bool sameFile( const QString& f1, const QString& f2 )
{
    QFile file1( f1 );
    QFile file2( f2 );
    if ( !file1.open( IO_ReadOnly ) || !file2.open( IO_ReadOnly ) )
        return false;
    if ( file1.size() != file2.size() )
        return false;
    QByteArray b1 = file1.readAll();
    QByteArray b2 = file2.readAll();
    file1.close();
    file2.close();
    return ( b1 == b2 );
}

void buildqmake()
{
    //# create the include and include/qmake directory (for qconfig.h and qmodules.h)
    QDir qtInstallPrefixDir ( options.install_prefix );
    QFile f;
    if ( qtInstallPrefixDir.exists( "include" ) ) {
        qtInstallPrefixDir.mkdir( "include/qmake" );
        f.setName( options.install_prefix + "/include/qmake/qconfig.h" );
        f.open( IO_WriteOnly | IO_Translate );
        char qconfigtext[] = "/* All features enabled while building qmake */\n";
        f.writeBlock( qconfigtext, sizeof( qconfigtext ) );
        f.close();
        f.setName( options.install_prefix + "/include/qmake/qmodules.h" );
        f.open( IO_WriteOnly | IO_Translate );
        char qmodulestext[] = "/* All modules enabled while building qmake */\n";
        f.writeBlock( qmodulestext, sizeof( qmodulestext ) );
        f.close();
    }

    QDir::setCurrent( options.install_prefix + "/qmake" );
    my_system( getMake() + " -f " + getMakefile() );
    QDir::setCurrent( options.install_prefix );

    //# clean up temporary qconfig.h and qmodules.h
    QFile::remove
        ( options.install_prefix + "/include/qmake/qconfig.h" );
    QFile::remove
        ( options.install_prefix + "/include/qmake/qmodules.h" );
    qtInstallPrefixDir.rmdir( options.install_prefix + "include/qmake" );
}


void callmake( const QString& folder )
{
    QString previous = QDir::current().absPath();
    QDir::setCurrent( folder );
    my_system( getMake() );
    QDir::setCurrent( previous );
}

void buildMakeSlow( const QString &file, QString &makefileName, QString &qmake_template,
                    bool createDSP )
{
    QString args = " -o " + makefileName;
    QString specarg = "-spec " + options.qmakespec;

    my_system( QDir::convertSeparators( options.install_prefix + "/bin/qmake " +
                                        specarg + " " + args + " " + file ) );

    if ( createDSP && !qmake_template.isEmpty() ) {
        args = "";
        my_system( QDir::convertSeparators( options.install_prefix + "/bin/qmake -t " +
                                            qmake_template + " " + specarg + " " +
                                            args + " " + file ) );
    }
}

void buildMakeFast( const QString &dir, const QString &file, QString &makefileName,
                    QString &qmake_template , bool createDSP )
{
    QString makefile = dir + "\\" + makefileName;
    QFile f( makefile );
    // fast configure - the makefile exists, skip it
    // since the makefile exists, it was generated by qmake, which means we
    // can skip it, since qmake has a rule to regenerate the makefile if the .pro
    // file changes...
    bool isBorlandMake = ( options.qmakespec == "win32-borland" );
    bool isMSVCMake = ( options.qmakespec.startsWith( "win32-msvc" ) );
    my_debug( "making " + makefile );

    if ( !f.open( IO_WriteOnly | IO_Truncate ) )
        qDebug( QString( "Error trying to open %1 for writing" ).arg( makefile ) );

    QTextStream s ( &f );

    s << "# " << makefile << ": generated by configure" << endl;
    s << "#" << endl;
    s << "# WARNING: This makefile will be replaced with a real makefile." << endl;
    s << "# All changes made to this file will be lost." << endl;
    s << endl;
    if ( isBorlandMake )
        s << "all: FORCE" << endl;
    else
        s << "all clean install qmake Makefile: FORCE" << endl;
    s << "\tcd " << QDir::convertSeparators(dir) << endl;
    s << "\t" << options.QT_INSTALL_BINS << "\\qmake -spec " << options.qmakespec << " -o " << makefileName << " " << file << endl;
    if ( isMSVCMake && !qmake_template.isEmpty() && createDSP ) {
        s << "\t" << options.QT_INSTALL_BINS << "\\qmake -spec " << options.qmakespec << " -t " << qmake_template
        << " " << file << endl;
    }
    s << "\t$(MAKE)" << endl;
    s << endl;
    s << "FORCE:" << endl;
    if ( isBorlandMake )
        s << "\t@cd ." << endl;
    s << "\t" << endl;
    s << endl;

    if ( isBorlandMake ) {
        // other targets
        s << "clean: all" << endl;
        s << "install: all" << endl;
        s << "qmake: all" << endl;
        s << "Makefile: all" << endl;
    }
    s << endl;

    f.close();
}

void buildmake( const QString& dir, bool createDSP )
{
    QStringList excludeList = QStringList::split( " ", "bin config.tests cygwin debian doc include misc mkspecs" );
    QString makefileName;

    QDir d ( dir );
    if ( !d.exists() ) {
        fprintf( out, "Can't read directory %s",
                 QDir::convertSeparators( dir ).latin1() );
        exit( 1 );
    }
    d.setFilter( QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoSymLinks );
    d.setNameFilter( "*.pro" );
    d.setSorting( QDir::Name );
    d.setMatchAllDirs( true );

    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    for ( ; ( fi = it.current() ) != 0; ++it ) {
        // skip "."  and ".." dir
        if ( fi->isDir() && ( fi->fileName() == "." || fi->fileName() == ".." ) )
            continue;
        else if ( excludeList.contains( fi->fileName() ) )
            continue;
        else if ( fi->isDir() )
            buildmake( d.absPath() + "/" + fi->fileName(), createDSP );
        else {
            QFile f( dir + "/" + fi->fileName() );
            if ( !f.open( IO_ReadOnly ) )
                qDebug( QString( "Error trying to open %1 for reading" ).arg( f.name() ) );
            QString line;
            QString qmake_template;
            bool allow_fast = true;
            QTextStream s ( &f );
            while ( !s.eof() && !f.atEnd() && qmake_template.isEmpty() ) {
                line = s.readLine();
                if ( line.contains( "TEMPLATE" ) && line.lower().contains( "app" ) )
                    qmake_template = "vcapp";
                else if ( line.contains( "TEMPLATE" ) && line.lower().contains( "lib" ) ) {
                    qmake_template = "vclib";
                    allow_fast = false;
                } else if ( line.contains( "TEMPLATE" ) && line.lower().contains( "subdirs" ) ) {
                    allow_fast = false;
                }
            }
            f.close();
            fprintf( out, QString( "For %1\n" ).arg( QDir::convertSeparators( f.name() ) ) );
            makefileName = ( fi->fileName() == "qtmain.pro" ) ? "Makefile.main" : "Makefile";
            QString pwd = QDir::current().path();
            QDir::setCurrent( fi->dirPath() );
            if ( options.fast && allow_fast ) {
                buildMakeFast( fi->dirPath(), fi->fileName(), makefileName, qmake_template, createDSP );
            } else {
                buildMakeSlow ( fi->fileName(), makefileName, qmake_template, createDSP );
            }
            QDir::setCurrent( pwd );
        }
    }
}

QStringList getAvailableStyle()
{
    QString stylesDir = options.install_prefix + "/src/styles";
    QDir d( stylesDir );
    d.setFilter( QDir::Files | QDir::NoSymLinks );
    d.setNameFilter( "q*style.cpp" );
    d.setSorting( QDir::Name );
    if ( !d.exists() && !options.rebuild ) {
        if ( !options.makeFiles ) {
            fprintf( out, "Can't read directory %s\n",
                     QDir::convertSeparators( stylesDir ).latin1() );
            exit( 1 );
        } else
            return QStringList();
    }

    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    QStringList styles;

    for ( ; ( fi = it.current() ) != 0; ++it ) {
#ifdef DONT_SUPPORT_WINDOWSXP_STYLE
    		if ( fi->fileName() == "qwindowsxpstyle.cpp" )
    			continue; 
#endif
        styles.append( fi->fileName().mid( 1, fi->fileName().length() - 10 ) );
    }
    return styles;
}

QStringList getAvailableSqlDriver()
{
    QString driversDir = options.install_prefix + "/plugins/src/sqldrivers";
    QDir d( driversDir );
    if ( !d.exists() && !options.rebuild ) {
        if ( !options.makeFiles ) {
            fprintf( out, "Can't read directory %s\n",
                     QDir::convertSeparators( driversDir ).latin1() );
            exit( 1 );
        } else
            return QStringList();
    }

    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    QStringList drivers;

    for ( ; ( fi = it.current() ) != 0; ++it ) {
        if ( fi->isDir() && !fi->fileName().startsWith( "." ) && fi->fileName() != "CVS" && fi->fileName() != "tmp" )
            drivers.append( fi->fileName() );
    }
    return drivers;
}

static struct
{
    const char* option[ 2 ];
    int prefered;
}
config_struct1[] =
    { { "shared", "staticlib", 0 },
      { "incremental", "no-incremental", 1 },
      { "release", "debug", 0 },
      { "thread", "no-thread", 0 },
      { "largefile", "no-largefile", 1 },
      { "gif", "no-gif", 1 },
      { "tablet", "no-tablet", 1 },
      { "exceptions", "no-exceptions", 0 },
      { "rtti", "no-rtti", 0 },
      { "pch", "no-pch", 1 },
      { "big-codecs", "no-big-codecs", 0 },
      { NULL, NULL, 0 } };

/* this are normally much more (see qglobal.h) */
static struct  // option 2 has to be the "no-xxxx"
{
    const char* option[ 2 ];
    const char* define;
    int prefered;
    int remove; // remove 'no-xxx' from qconfig to reduce the qconfig output
}
config_struct2[] =
    { { "stl", "no-stl", "QT_NO_STL", 0, 1 },
      { "ipv6", "no-ipv6", "QT_NO_IPV6", 0, 1 },
      { "zlib", "no-zlib", "QT_NO_COMPRESS", 0, 0 },
      { "opengl", "no-opengl", "QT_NO_OPENGL", 1, 0 },
      { "sound", "no-sound", "QT_NO_SOUND", 0, 0 },
      { NULL, NULL, NULL, 0, 0 } };

/* Check for opposed config options and insert standard values if none set */
void CheckOptionsQConfig()
{
    int i;
    for ( i = 0; config_struct1[ i ].option[ 0 ]; i++ ) {
        int pref = config_struct1[ i ].prefered;
        if ( ( pref < 0 ) || ( pref > 2 ) )
            pref = 0;
        const char *option1 = config_struct1[ i ].option[ 0 ];
        const char *option2 = config_struct1[ i ].option[ 1 ];
        const char *prefStr = config_struct1[ i ].option[ pref ];

        /* check if both options are given */
        if ( options.QConfig.contains( option1 ) &&
                options.QConfig.contains( option2 ) ) {
            /* Remove the value which is prefered since the other was given by command line */
            options.QConfig.remove( prefStr );
        }
        /* check if none of the two options were given */
        if ( !options.QConfig.contains( option1 ) &&
                !options.QConfig.contains( option2 ) ) {
            /* Append standard option */
            options.QConfig.append( prefStr );
        }
    }
    for ( i = 0; config_struct2[ i ].option[ 0 ]; i++ ) {
        int pref = config_struct2[ i ].prefered;
        if ( ( pref < 0 ) || ( pref > 2 ) )
            pref = 0;
        const char *option1 = config_struct2[ i ].option[ 0 ];
        const char *option2 = config_struct2[ i ].option[ 1 ];
        const char *prefStr = config_struct2[ i ].option[ pref ];

        /* check if both options are given */
        if ( options.QConfig.contains( option1 ) &&
                options.QConfig.contains( option2 ) ) {
            /* Remove the value which is prefered since the other was given by command line */
            options.QConfig.remove( prefStr );
        }
        /* check if none of the two options were given */
        if ( !options.QConfig.contains( option1 ) &&
                !options.QConfig.contains( option2 ) ) {
            /* Append standard option */
            options.QConfig.append( prefStr );
        }
        if ( options.QConfig.contains( option2 ) ) {
            if ( config_struct2[ i ].remove )
                options.QConfig.remove ( option2 );
            options.QCFlags.append ( config_struct2[ i ].define );
        }
    }
    /* pch has to be precompile_header */
    if ( options.QConfig.contains( "pch" ) ) {
        options.QConfig.remove( "pch" );
        options.QConfig.append( "precompile_header" );
    }
    /* big-codecs is bigcodecs */
    if ( options.QConfig.contains( "big-codecs" ) ) {
        options.QConfig.remove( "big-codecs" );
        options.QConfig.append( "bigcodecs" );
    }
}

QString CheckDir( QString arg, QString type )
{
    QDir dir( arg );
    if ( !dir.exists() ) {
        qWarning( QString( "WARNING: The specified path %1 for %2 doesn't exist" ).arg( QDir::convertSeparators ( dir.absPath() ) ).arg ( type ) );
        exit ( 1 );
    }
    return ( QDir::convertSeparators( dir.absPath() ) );
}

void parseCommandLine( const QStringList & argv )
{
    // initialization
    options.install_prefix = getenv( "QTDIR" );
    if ( options.install_prefix.length() == 0 ) {
        fprintf( out, "QTDIR not set!" );
        exit ( 1 );
    }
    QDir qtdir( options.install_prefix );
    if ( !qtdir.exists() ) {
        fprintf( out, "QTDIR %s doesn't exist!", options.install_prefix.latin1() );
        exit ( 1 );
    }

    // must be set before getAvailable ...()
    for ( uint j = 1; j < argv.size(); ++j ) {
        QString arg( argv[ j ] );
        arg.remove( QRegExp( "^(-)*" ) );
        if ( arg == "make-files" )
            options.makeFiles = true;
    }

    options.install_prefix = qtdir.absPath();   // remove possible '\\' at end of path since this confuses qmake

    options.QModules = QStringList::split( " ", "styles tools thread kernel widgets dialogs iconview workspace network canvas table xml opengl sql accessibility tablet sound" );
    options.QNoModules = QStringList::split( " ", "" );
    options.QConfig = QStringList::split( " ", "enterprise nocrosscompiler stl rtti pch warn_off create_prl link_prl minimal-config small-config medium-config large-config full-config" );
    options.QStyles = options.QStylesAvailable = getAvailableStyle();
    options.QImageFmts = options.QImageFmtsAvailable = QStringList::split( " " , "png mng jpeg" );
    options.sqlDriverAvailable = getAvailableSqlDriver();
    options.build = true;

    QString configStatus = "";

    options.qmakespec = QString( getenv( "QMAKESPEC" ) ).lower();

    // Enabled by default when using compilers that use Visual Studio Project files ;)
    if ( options.qmakespec.startsWith( "win32-msvc" ) || options.qmakespec == "win32-icc" )
        options.createDSPFiles = true;
    else
        options.createDSPFiles = false;

    for ( uint i = 1; i < argv.size(); ++i ) {
        configStatus += argv[ i ] + QString( " " );
        QString arg( argv[ i ] );
        QString tmp;
        arg.remove( QRegExp( "^(-)*" ) );
        my_debug( arg.ascii() );

        if ( arg == "help" ) {
            usage();
            exit( 1 );
        } else if ( arg == "v" || arg == "verbose" ) {
            options.verbose = true;
        } else if ( arg == "fast" ) {
            options.fast = true;
        } else if ( arg == "make-files" ) {
            options.makeFiles = true;
        } else if ( arg == "rebuild" ) {
            options.rebuild = true;
        } else if ( arg == "dryrun" || arg == "n" ) {
            options.dryrun = true;
        } else if ( arg == "no-dsp" ) {
            options.createDSPFiles = false;
        } else if ( arg == "dsp" ) {
            options.createDSPFiles = true;
            /* Styles */
        } else if ( arg.startsWith( "qt-style-" ) ) {
            tmp = arg.remove( "qt-style-" ).stripWhiteSpace();
            if ( !options.QStylesAvailable.contains( tmp ) ) {
                qWarning( "Specified style %s doesn't exist", tmp.latin1() );
                exit ( 1 );
            }
            options.QStyles.append( tmp );
            options.QStylesNo.remove( tmp );
            options.QStylesPlugin.remove( tmp );
        } else if ( arg.startsWith( "plugin-style-" ) ) {
            tmp = arg.remove( "plugin-style-" ).stripWhiteSpace();
            if ( !options.QStylesAvailable.contains( tmp ) ) {
                qWarning( "Specified style %s doesn't exist", tmp.latin1() );
                exit ( 1 );
            }
            options.QStyles.remove( tmp );
            options.QStylesNo.remove( tmp );
            options.QStylesPlugin.append( tmp );
        } else if ( arg.startsWith( "no-style-" ) ) {
            tmp = arg.remove( "no-style-" ).stripWhiteSpace();
            if ( !options.QStylesAvailable.contains( tmp ) ) {
                qWarning( "Specified style %s doesn't exist", tmp.latin1() );
                exit ( 1 );
            }
            options.QStyles.remove( tmp );
            options.QStylesNo.append( tmp );
            options.QStylesPlugin.remove( tmp );
            /* Imageformats */
        } else if ( arg.startsWith( "qt-imgfmt-" ) ) {
            tmp = arg.remove( "qt-imgfmt-" ).stripWhiteSpace();
            if ( !options.QImageFmtsAvailable.contains( tmp ) ) {
                qWarning( "Specified imageformat %s doesn't exist", tmp.latin1() );
                exit ( 1 );
            }
            options.QImageFmts.append( tmp );
            options.QImageFmtsNo.remove( tmp );
            options.QImageFmtsPlugin.remove( tmp );
        } else if ( arg.startsWith( "plugin-imgfmt-" ) ) {
            tmp = arg.remove( "plugin-imgfmt-" ).stripWhiteSpace();
            if ( !options.QImageFmtsAvailable.contains( tmp ) ) {
                qWarning( "Specified imageformat %s doesn't exist", tmp.latin1() );
                exit ( 1 );
            }
            options.QImageFmts.remove( tmp );
            options.QImageFmtsNo.remove( tmp );
            options.QImageFmtsPlugin.append( tmp );
        } else if ( arg.startsWith( "no-imgfmt-" ) ) {
            tmp = arg.remove( "no-imgfmt-" ).stripWhiteSpace();
            if ( !options.QImageFmtsAvailable.contains( tmp ) ) {
                qWarning( "Specified imageformat %s doesn't exist", tmp.latin1() );
                exit ( 1 );
            }
            options.QImageFmts.remove( tmp );
            options.QImageFmtsNo.append( tmp );
            options.QImageFmtsPlugin.remove( tmp );
            /* Modules */
        } else if ( arg.startsWith( "disable-" ) ) {
            tmp = arg.remove( "disable-" ).stripWhiteSpace();
            options.QModules.remove( tmp );
            options.QNoModules.append( tmp );
            /* SQL */
        } else if ( arg.startsWith( "qt-sql-" ) ) {
            tmp = arg.remove( "qt-sql-" ).stripWhiteSpace();
            if ( !options.sqlDriverAvailable.contains( tmp ) ) {
                qWarning( "Specified sql-driver %s doesn't exist", tmp.latin1() );
                exit ( 1 );
            }
            options.sqlDriver.append( tmp );
            options.sqlDriverNo.remove( tmp );
            options.sqlDriverPlugin.remove( tmp );
        } else if ( arg.startsWith( "plugin-sql-" ) ) {
            tmp = arg.remove( "plugin-sql-" ).stripWhiteSpace();
            if ( !options.sqlDriverAvailable.contains( tmp ) ) {
                qWarning( "Specified sql-driver %s doesn't exist", tmp.latin1() );
                exit ( 1 );
            }
            options.sqlDriver.remove( tmp );
            options.sqlDriverNo.remove( tmp );
            options.sqlDriverPlugin.append( tmp );
        } else if ( arg.startsWith( "no-sql-" ) ) {
            tmp = arg.remove( "no-sql-" ).stripWhiteSpace();
            if ( !options.sqlDriverAvailable.contains( tmp ) ) {
                qWarning( "Specified sql-driver %s doesn't exist", tmp.latin1() );
                exit ( 1 );
            }
            options.sqlDriver.remove( tmp );
            options.sqlDriverNo.append( tmp );
            options.sqlDriverPlugin.remove( tmp );
        } else if ( arg == "prefix" ) {
            options.install_prefix = CheckDir ( argv[ ++i ], arg );
        } else if ( arg == "bindir" ) {
            options.QT_INSTALL_BINS = CheckDir ( argv[ ++i ], arg );
        } else if ( arg == "libdir" ) {
            options.QT_INSTALL_LIBS = CheckDir ( argv[ ++i ], arg );
        } else if ( arg == "headerdir" ) {
            options.QT_INSTALL_HEADERS = CheckDir ( argv[ ++i ], arg );
        } else if ( arg == "docdir" ) {
            options.QT_INSTALL_DOCS = CheckDir ( argv[ ++i ], arg );
        } else if ( arg == "plugindir" ) {
            options.QT_INSTALL_PLUGINS = CheckDir ( argv[ ++i ], arg );
        } else if ( arg == "datadir" ) {
            options.QT_INSTALL_DATA = CheckDir ( argv[ ++i ], arg );
        } else if ( arg == "translationdir" ) {
            options.QT_INSTALL_TRANSLATIONS = CheckDir ( argv[ ++i ], arg );
        } else if ( arg == "sysconfdir" ) {
            options.QT_INSTALL_SYSCONF = CheckDir ( argv[ ++i ], arg );
        } else if ( arg == "release" || arg == "debug" || arg == "shared" ||
                    arg == "incremental" || arg == "no-incremental" ||
                    arg == "thread" || arg == "no-thread" || arg == "largefile" || arg == "no-largefile" ||
                    arg == "gif" || arg == "no-gif" ||
                    arg == "zlib" || arg == "no-zlib" || arg == "opengl" || arg == "no-opengl" ||
                    arg == "tablet" || arg == "no-tablet" || arg == "exceptions" || arg == "no-exceptions" ||
                    arg == "rtti" || arg == "no-rtti" || arg == "stl" || arg == "no-stl" ||
                    arg == "pch" || arg == "no-pch" || arg == "ipv6" || arg == "no-ipv6" ||
                    arg == "sound" || arg == "no-sound" || arg == "big-codecs" || arg == "no-big-codecs" )
            options.QConfig.append( arg );
        else if ( arg == "static" )
            options.QConfig.append( "staticlib" );
        else if ( arg == "spec" )
            options.qmakespec = argv[ ++i ];
        else if ( arg == "log" ) {
            options.logfile = strdup( argv[ ++i ] );
        } else if ( arg == "build" ) {
            options.build = true;
        } else if ( arg == "nobuild" ) {
            options.build = false;
        } else {
            qWarning( QString( "Ops, unrecognized option [%1]\n" ).arg( arg ) );
            exit ( 1 );
        }
    }
    CheckOptionsQConfig();
}

void deletePlugins( QString dir )
{
    QDir d ( dir );
    if ( !d.exists() ) {
        fprintf( out, "Can't read directory %s",
                 QDir::convertSeparators( dir ).latin1() );
        exit( 1 );
    }
    d.setFilter( QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoSymLinks );
    d.setNameFilter( "*.dll *.lib *.exp *.pdb" );
    d.setSorting( QDir::Name );
    d.setMatchAllDirs( true );

    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    for ( ; ( fi = it.current() ) != 0; ++it ) {
        // skip "."  and ".." dir
        if ( fi->isDir() && ( fi->fileName() == "." || fi->fileName() == ".." ) )
            continue;
        else if ( fi->isDir() )
            deletePlugins( d.absPath() + "/" + fi->fileName() );
        else {
            QFile f( dir + "/" + fi->fileName() );
            QString str = "deleting " + dir + "\\" + fi->fileName();
            my_debug( str );
            f.remove();
        }
    }
}

void doBuild()
{
    fprintf( out, "Creating makefiles. Please wait...\n" );
    buildmake( options.install_prefix, options.createDSPFiles );

    if ( options.build ) {
        fprintf( out, "Building qt...\n" );
        callmake( options.install_prefix );
    }
}

void makeFiles()
{
    QString buildDir = QDir::current().absPath();

    fprintf( out, "Creating makefiles. Please wait...\n" );
    buildmake( buildDir, options.createDSPFiles );

    fprintf( out, "Building package ...\n" );
    callmake( buildDir );
}

void WriteUnusedImageformats( QTextStream &s, QString format )
{
    if ( options.QConfig.contains( "no-" + format.lower() ) ) {
        s << "#ifndef QT_NO_IMAGEIO_" << format.upper() << "\n";
        s << "# define QT_NO_IMAGEIO_" << format.upper() << "\n";
        s << "#endif\n\n";
    }
}

void doConfigure()
{
    //#-------------------------------------------------------------------------------
    //# post initialize QT_INSTALL_* variables, and generate qconfig.cpp
    //#-------------------------------------------------------------------------------

    options.install_prefix = QDir::convertSeparators( options.install_prefix );
    if ( options.QT_INSTALL_DOCS.isEmpty() )
        options.QT_INSTALL_DOCS = QDir::convertSeparators( options.install_prefix + "/doc" );
    if ( options.QT_INSTALL_HEADERS.isEmpty() )
        options.QT_INSTALL_HEADERS = QDir::convertSeparators( options.install_prefix + "/include" );
    if ( options.QT_INSTALL_LIBS.isEmpty() )
        options.QT_INSTALL_LIBS = QDir::convertSeparators( options.install_prefix + "/lib" );
    if ( options.QT_INSTALL_BINS.isEmpty() )
        options.QT_INSTALL_BINS = QDir::convertSeparators( options.install_prefix + "/bin" );
    if ( options.QT_INSTALL_PLUGINS.isEmpty() )
        options.QT_INSTALL_PLUGINS = QDir::convertSeparators( options.install_prefix + "/plugins" );
    if ( options.QT_INSTALL_DATA.isEmpty() )
        options.QT_INSTALL_DATA = QDir::convertSeparators( options.install_prefix );
    if ( options.QT_INSTALL_TRANSLATIONS.isEmpty() )
        options.QT_INSTALL_TRANSLATIONS = QDir::convertSeparators( options.install_prefix + "/translations" );
    if ( options.QT_INSTALL_SYSCONF.isEmpty() )
        options.QT_INSTALL_SYSCONF = QDir::convertSeparators( options.install_prefix );

    // Give feedback

#define HasConfig(x)   QString ( options.QConfig.contains(x) && !options.QConfig.contains("no-"x) ? "yes" : "no" ) + "\n"
#define HasFlag(x)     QString ( options.x ? "yes" : "no" ) + "\n"
#define HasImageFmt(x) QString ( options.QImageFmtsNo.contains(x) ? "no" : "yes" ) + "\n"

#define ifConfig(x) (options.QConfig.contains(x) && !options.QConfig.contains("no-"x) ? "yes" : "no")

    fprintf( out, "Incremental ......... " + HasConfig( "incremental" ) );
    fprintf( out, "Configuration ....... " + options.QConfig.join( " " ) + "\n" );
    fprintf( out, "STL support ......... " + HasConfig( "stl" ) );
    fprintf( out, "PCH support ......... " + HasConfig( "precompile_header" ) );
    fprintf( out, "IPv6 support ........ " + HasConfig( "ipv6" ) );
    fprintf( out, "Thread support ...... " + HasConfig( "thread" ) );
    fprintf( out, "Debug symbols........ " + HasConfig( "debug" ) );
    fprintf( out, "verbose mode......... " + HasFlag( verbose ) );
    fprintf( out, "fast mode............ " + HasFlag( fast ) );

    fprintf( out, "Large File support .. " + HasConfig( "largefile" ) );
    fprintf( out, "QMAKESPEC............ " + options.qmakespec );
    fprintf( out, "\n" );
    fprintf( out, "GIF support ......... " + HasConfig( "gif" ) );
    fprintf( out, "PNG support ......... " + HasImageFmt( "png" ) );
    fprintf( out, "MNG support ......... " + HasImageFmt( "mng" ) );
    fprintf( out, "JPEG support ........ " + HasImageFmt( "jpeg" ) );
    fprintf( out, "zlib support ........ " + HasConfig( "zlib" ) );
    fprintf( out, "\n" );
    fprintf( out, "OpenGL support ...... " + HasConfig( "opengl" ) );
    fprintf( out, "Tablet support ...... " + HasConfig( "tablet" ) );
    fprintf( out, "Exception support.... " + HasConfig( "exceptions" ) );
    fprintf( out, "RTTI support......... " + HasConfig( "rtti" ) );
    fprintf( out, "SOUND support ....... " + HasConfig( "sound" ) );
    fprintf( out, "\n" );
    fprintf( out, "SQL driver  ......... " + options.sqlDriver.join( " " ) + "\n" );
    fprintf( out, "SQL driver plugins .. " + options.sqlDriverPlugin.join( " " ) + "\n" );
    fprintf( out, "\n" );
    fprintf( out, "Styles .............. " + options.QStyles.join( " " ) + "\n" );
    fprintf( out, "Style plugins ....... " + options.QStylesPlugin.join( " " ) + "\n" );
    fprintf( out, "\n" );
    fprintf( out, "Install prefix....... " + options.install_prefix + "\n" );
    fprintf( out, "Headers path......... " + options.QT_INSTALL_HEADERS + "\n" );
    fprintf( out, "Libraries path....... " + options.QT_INSTALL_LIBS + "\n" );
    fprintf( out, "Binaries path........ " + options.QT_INSTALL_BINS + "\n" );
    fprintf( out, "Plugins path......... " + options.QT_INSTALL_PLUGINS + "\n" );

    if ( options.QConfig.contains( "staticlib" ) )
        fprintf( out, "\n"
                 "WARNING: Using static linking will disable the use of plugins."
                 "Make sure you compile ALL needed modules into the library."
                 "\n" );

#undef HasConfig

    fprintf( out, "\n\n\n" );

    // # generate qconfig.cpp
    QFile f( options.install_prefix + "/src/tools/qconfig.cpp.new" );
    if ( f.open( IO_WriteOnly | IO_Translate ) ) {

        fprintf( out, "generating qconfig.cpp\n" );
        QTextStream s( &f );
        s << QString( "#include <qglobal.h>\n"
                      "#include <qcstring.h>\n"
                      "#include <string.h>\n"
                      "#include <stdlib.h>\n"

                      "/* Install paths from configure */\n"
                      "\n"
                      "static char install_prefix         [267];\n"
                      "static char QT_INSTALL_BINS        [267];\n"
                      "static char QT_INSTALL_DOCS        [267];\n"
                      "static char QT_INSTALL_HEADERS     [267];\n"
                      "static char QT_INSTALL_LIBS        [267];\n"
                      "static char QT_INSTALL_PLUGINS     [267];\n"
                      "static char QT_INSTALL_DATA        [267];\n"
                      "static char QT_INSTALL_TRANSLATIONS[267];\n"
                      "static char QT_INSTALL_SYSCONF     [267];\n"
                      "\n"
                      "/* strlen( \"qt_xxxpath=\" ) == 11 */\n"
                      "Q_EXPORT const char *qInstallPath()             { return install_prefix          + 11; }\n"
                      "Q_EXPORT const char *qInstallPathDocs()         { return QT_INSTALL_DOCS         + 11; }\n"
                      "Q_EXPORT const char *qInstallPathHeaders()      { return QT_INSTALL_HEADERS      + 11; }\n"
                      "Q_EXPORT const char *qInstallPathLibs()         { return QT_INSTALL_LIBS         + 11; }\n"
                      "Q_EXPORT const char *qInstallPathBins()         { return QT_INSTALL_BINS         + 11; }\n"
                      "Q_EXPORT const char *qInstallPathPlugins()      { return QT_INSTALL_PLUGINS      + 11; }\n"
                      "Q_EXPORT const char *qInstallPathData()         { return QT_INSTALL_DATA         + 11; }\n"
                      "Q_EXPORT const char *qInstallPathTranslations() { return QT_INSTALL_TRANSLATIONS + 11; }\n"
                      "Q_EXPORT const char *qInstallPathSysconf()      { return QT_INSTALL_SYSCONF      + 11; }\n"
                      "\n"
                      "class QConfigInit {\n"
                      "public:\n"
                      " QConfigInit()\n"
                      " { \n"
                      "  QCString qtdir = getenv(\"QTDIR\");\n"
                      "  qtdir.replace(\"\\\\\",\"/\");\n"
                      "  QCString a;\n"
                      "  a = QCString(\"qt_nstpath=\") + qtdir;                   strcpy(install_prefix         ,a.data()); \n"
                      "  a = QCString(\"qt_binpath=\") + qtdir + \"/bin\";          strcpy(QT_INSTALL_BINS        ,a.data()); \n"
                      "  a = QCString(\"qt_docpath=\") + qtdir + \"/doc\";          strcpy(QT_INSTALL_DOCS        ,a.data()); \n"
                      "  a = QCString(\"qt_hdrpath=\") + qtdir + \"/include\";      strcpy(QT_INSTALL_HEADERS     ,a.data()); \n"
                      "  a = QCString(\"qt_libpath=\") + qtdir + \"/lib\";          strcpy(QT_INSTALL_LIBS        ,a.data()); \n"
                      "  a = QCString(\"qt_plgpath=\") + qtdir + \"/plugins\";      strcpy(QT_INSTALL_PLUGINS     ,a.data()); \n"
                      "  a = QCString(\"qt_datpath=\") + qtdir;                   strcpy(QT_INSTALL_DATA        ,a.data()); \n"
                      "  a = QCString(\"qt_trnpath=\") + qtdir + \"/translations\"; strcpy(QT_INSTALL_TRANSLATIONS,a.data()); \n"
                      "  a = QCString(\"qt_cnfpath=\") + qtdir;                   strcpy(QT_INSTALL_SYSCONF     ,a.data()); \n"
                      " } \n"
                      "};\n"
                      "QConfigInit x;\n"
                    );
    }
    f.close();

    // Avoid rebuilds checking if it didn't change.
    QDir qtInstallPrefixDir ( options.install_prefix );
    if ( !sameFile( options.install_prefix + "/src/tools/qconfig.cpp.new", options.install_prefix + "/src/tools/qconfig.cpp" ) ) {
        fprintf( out, "activating qconfig.cpp\n" );
        QFile::remove
            ( options.install_prefix + "/src/tools/qconfig.cpp" );
        qtInstallPrefixDir.rename( options.install_prefix + "/src/tools/qconfig.cpp.new", options.install_prefix + "/src/tools/qconfig.cpp" );
        // TODO: check if original file is readonly, in this case the config isn't updated
    } else
        QFile::remove
            ( options.install_prefix + "/src/tools/qconfig.cpp.new" );

    //# build up the variables for output
    //#-------------------------------------------------------------------------------
    //# part of configuration information goes into qmodules.h
    //#-------------------------------------------------------------------------------

    f.setName( options.install_prefix + "/include/qmodules.h.new" );
    if ( f.open( IO_WriteOnly | IO_Translate ) ) {
        QTextStream s( &f );
        for ( uint i = 0; i < options.QModules.count(); ++i ) {
            s << "#define QT_MODULE_" << options.QModules[ i ].upper() << "\n";
        }
        f.close();
    }

    // Avoid rebuilds checking if it didn't change.
    if ( !sameFile( options.install_prefix + "/include/qmodules.h.new", options.install_prefix + "/include/qmodules.h" ) ) {
        QFile::remove
            ( options.install_prefix + "/include/qmodules.h" );
        qtInstallPrefixDir.rename( options.install_prefix + "/include/qmodules.h.new", options.install_prefix + "/include/qmodules.h" );
        // TODO: check if original file is readonly, in this case the config isn't updated
    } else
        QFile::remove
            ( options.install_prefix + "/include/qmodules.h.new" );

    //#-------------------------------------------------------------------------------
    //# part of configuration information goes into qconfig.h
    //#-------------------------------------------------------------------------------

    f.setName( options.install_prefix + "/include/qconfig.h.new" );
    if ( f.open( IO_WriteOnly | IO_Translate ) ) {
        int i;

        QTextStream s( &f );
        s << "/* License information */\n";
        s << "#define QT_PRODUCT_LICENSEE \"Free\"\n";
        s << "#define QT_PRODUCT_LICENSE  \"qt-free\"\n";
        s << "\n";
        s << "/* Build key */\n";
        s << "#define QT_BUILD_KEY \"" << getenv( "QMAKESPEC" ) << "\"\n";
        s << "\n\n/* Machine byte-order */\n";
        s << "#define Q_BIG_ENDIAN 4321\n";
        s << "#define Q_LITTLE_ENDIAN 1234\n";
        s << "#define Q_BYTE_ORDER Q_LITTLE_ENDIAN\n";

        s << "\n/* Compile time features */\n";
        s << "#define QT_POINTER_SIZE " << static_cast<int>( sizeof( void* ) ) << "\n";
        for ( i = 0; i < options.QStylesNo.count(); ++i ) {
            s << "#ifndef QT_NO_STYLE_" << options.QStylesNo[ i ].upper() << "\n";
            s << "# define QT_NO_STYLE_" << options.QStylesNo[ i ].upper() << "\n";
            s << "#endif\n\n";
        }
        WriteUnusedImageformats( s, "png" );
        WriteUnusedImageformats( s, "gif" );
        WriteUnusedImageformats( s, "mng" );
        WriteUnusedImageformats( s, "jpeg" );
        for ( i = 0; i < options.QCFlags.count(); i++ ) {
            s << "#ifndef " << options.QCFlags[ i ].upper() << "\n";
            s << "# define " << options.QCFlags[ i ].upper() << "\n";
            s << "#endif\n\n";
        }
        s << "#ifndef QT_QWINEXPORT" << endl;
        s << "#define QT_QWINEXPORT" << endl;
        s << "#endif" << endl;
        if ( !options.QConfig.contains( "staticlib" ) ) {
            s << "#ifndef QT_DLL" << endl;
            s << "#define QT_DLL" << endl;
            s << "#endif" << endl;
        }
        f.close();
    }

    // Avoid rebuilds checking if it didn't change.
    if ( !sameFile( options.install_prefix + "/include/qconfig.h.new", options.install_prefix + "/include/qconfig.h" ) ) {
        QFile::remove
            ( options.install_prefix + "/include/qconfig.h" );
        qtInstallPrefixDir.rename( options.install_prefix + "/include/qconfig.h.new", options.install_prefix + "/include/qconfig.h" );
        /* When qconfig.h changed, we have to delete the plugins */
        deletePlugins( options.QT_INSTALL_PLUGINS );

    } else
        QFile::remove
            ( options.install_prefix + "/include/qconfig.h.new" );

    // Build .qmake.cache & .qtwinconfig
    // qmake only reads .qtwinconfig on *every* run and so there must be all
    // config options like 'shared' or release
    // .qmake.cache is only read when qmake is called in a subdir of <qtdir>

    f.setName ( options.install_prefix + "/.qtwinconfig" );
    if ( f.open( IO_WriteOnly | IO_Translate ) ) {
        /* only shared/static, thread, release/debug, stl, rtti and exceptions */
        QTextStream s( &f );
        s << "CONFIG += ";
        if ( options.QConfig.contains( "shared" ) )
            s << "shared ";
        else if ( options.QConfig.contains( "staticlib" ) )
            s << "static ";

        if ( options.QConfig.contains( "thread" ) )
            s << "thread ";

        if ( options.QConfig.contains( "debug" ) )
            s << "debug ";
        else if ( options.QConfig.contains( "release" ) )
            s << "release ";

        if ( options.QConfig.contains( "stl" ) )
            s << "stl ";

        if ( options.QConfig.contains( "rtti" ) )
            s << "rtti ";

        if ( options.QConfig.contains( "exceptions" ) )
            s << "exceptions ";

        s << endl;

        f.close();
    }
    f.setName ( options.install_prefix + "/.qmake.cache" );
    if ( f.open( IO_WriteOnly | IO_Translate ) ) {
        QTextStream s( &f );
        QString QMAKE_OUTDIR;
        if ( options.QConfig.contains( "debug" ) )
            QMAKE_OUTDIR += "debug";
        else if ( options.QConfig.contains( "release" ) )
            QMAKE_OUTDIR += "release";
        if ( options.QConfig.contains( "shared" ) )
            QMAKE_OUTDIR += "-shared";
        else if ( options.QConfig.contains( "staticlib" ) )
            QMAKE_OUTDIR += "-static";
        if ( options.QConfig.contains( "thread" ) )
            QMAKE_OUTDIR += "-mt";

        s << "QMAKE_QT_VERSION_OVERRIDE=" << QString::number( ( QT_VERSION >> 16 ) & 0xff ) << "\n";
        s << "OBJECTS_DIR = tmp/obj/" << QMAKE_OUTDIR << "\n";
        s << "MOC_DIR = tmp/moc/" << QMAKE_OUTDIR << "\n";
        s << "DEFINES += \n";
        s << "INCLUDEPATH += \n";
        s << "sql-drivers += " << options.sqlDriver.join( " " ) << "\n";
        s << "sql-plugins += " << options.sqlDriverPlugin.join( " " ) << "\n";
        s << "styles += " << options.QStyles.join( " " ) << "\n";
        s << "style-plugins += " << options.QStylesPlugin.join( " " ) << "\n";
        s << "imageformat-plugins += " << options.QImageFmtsPlugin.join( " " ) << "\n";
        s << "QT_PRODUCT=qt-free\n";
        s << "CONFIG += " << options.QConfig.join( " " ) << " "
        << options.QModules.join( " " ) << " "
        << options.QImageFmts.join( " " ) << "\n";
        s << "QMAKESPEC=" << options.qmakespec << "\n";
        s << "QT_BUILD_TREE = " << options.install_prefix << "\n";
        s << "QT_SOURCE_TREE = " << options.install_prefix << "\n";
        s << "QT_INSTALL_PREFIX = " << options.install_prefix << "\n";
        s << "QT_INSTALL_TRANSLATIONS = " << options.QT_INSTALL_TRANSLATIONS << "\n";
        s << "QMAKE_LIBDIR_QT = " << options.QT_INSTALL_LIBS << "\n";
        s << "docs.path = " << options.QT_INSTALL_DOCS << "\n";
        s << "headers.path = " << options.QT_INSTALL_HEADERS << "\n";
        s << "plugins.path = " << options.QT_INSTALL_PLUGINS << "\n";
        s << "libs.path = " << options.QT_INSTALL_LIBS << "\n";
        s << "bins.path = " << options.QT_INSTALL_BINS << "\n";
        s << "data.path = " << options.QT_INSTALL_DATA << "\n";
        s << "translations.path = " << options.QT_INSTALL_TRANSLATIONS << "\n";
        s << "CONFIG -= " << " " << "\n";

        f.close();
    }

    fprintf( out, "Building qmake\n" );
    buildqmake();

    //#-------------------------------------------------------------------------------
    //# build makefiles based on the configuration
    //#-------------------------------------------------------------------------------

    doBuild();
}

void usage( void )
{
    fprintf( out, "Usage:  configure [-prefix dir] [-buildkey key] [-docdir dir] [-headerdir dir] \n" );
    fprintf( out, " [-libdir dir] [-bindir dir] [-plugindir dir ] [-datadir dir]                  \n" );
    fprintf( out, " [-translationdir dir] [-sysconfdir dir] [-debug] [-release]                   \n" );
    fprintf( out, " [-no-gif] [-qt-gif] [-no-sm] [-sm] [-qt-zlib] [-system-zlib]                  \n" );
    fprintf( out, " [-qt-libjpeg] [-system-libjpeg] [-qt-libpng] [-system-libpng]                 \n" );
    fprintf( out, " [-qt-libmng] [-system-libmng] [-no-thread] [-thread]                          \n" );
    fprintf( out, " [-no-largefile] [-largefile]                                                  \n" );
    fprintf( out, " [-no-stl] [-stl] [-no-ipv6] [-ipv6] [-Istring] [-lstring]                     \n" );
    fprintf( out, " [-Lstring] [-Rstring] [-disable-<module>] [-with-<module setting>]            \n" );
    fprintf( out, " [-without-<module setting>] [-fast] [-no-fast]                                \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, "Installation options:                                                          \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, " These are optional, but you may specify install directories.                  \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, "    -prefix dir ........ This will install everything relative dir             \n" );
    fprintf( out, "     (default $QT_INSTALL_PREFIX)                                            \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, " You may use these to separate different parts of the install:                 \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, "    -bindir dir ........ Executables will be installed to dir                  \n" );
    fprintf( out, "     (default PREFIX/bin)                                                    \n" );
    fprintf( out, "    -libdir dir ........ Libraries will be installed to dir                    \n" );
    fprintf( out, "     (default PREFIX/lib)                                                    \n" );
    fprintf( out, "    -docdir dir ........ Documentation will be installed to dir                \n" );
    fprintf( out, "     (default PREFIX/doc)                                                    \n" );
    fprintf( out, "    -headerdir dir ..... Headers will be installed to dir                      \n" );
    fprintf( out, "     (default PREFIX/include)                                                \n" );
    fprintf( out, "    -plugindir dir ..... Plugins will be installed to dir                      \n" );
    fprintf( out, "                          (default PREFIX/plugins)                             \n" );
    fprintf( out, "    -datadir dir ....... Data used by Qt programs will be installed to dir     \n" );
    fprintf( out, "                          (default PREFIX)                                     \n" );
    fprintf( out, "    -translationdir dir  Translations of Qt programs will be installed to dir  \n" );
    fprintf( out, "                          (default PREFIX/translations)                        \n" );
    fprintf( out, "    -sysconfdir dir ...  Settings used by Qt programs will be looked for in dir\n" );
    fprintf( out, "                          (default PREFIX/etc/settings)                        \n" );
    fprintf( out, "                                                                               \n" );
#if 0

    fprintf( out, " You may use these options to turn on strict plugin loading.                   \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, "    -buildkey key ...... Build the Qt library and plugins using the specified  \n" );
    fprintf( out, "    key.  When the library loads plugins, it will only                         \n" );
    fprintf( out, "    load those that have a matching key.                                       \n" );
    fprintf( out, "                                                                               \n" );
#endif

    fprintf( out, "Configure options:                                                             \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, " The defaults (*) are usually acceptable.  If marked with a plus (+) a test    \n" );
    fprintf( out, " for that feature has not been done yet, but will be evaluated later, the      \n" );
    fprintf( out, " plus simply denotes the default value. Here is a short explanation of each    \n" );
    fprintf( out, " option:                                                                       \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, " *  -incremental ....... Use incremental linking.                              \n" );
    fprintf( out, "    -no-incremental .... Don't use incremenatal linking.                       \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, " *  -release ........... Compile and link Qt with debugging turned off.        \n" );
    fprintf( out, "    -debug ............. Compile and link Qt with debugging turned on.         \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, " *  -shared ............ Create and use a shared Qt library (libqt.so).        \n" );
    fprintf( out, "    -static ............ Create and use a static Qt library (libqt.a).         \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, "    -fast .............. Configure Qt quickly by generating Makefiles only for \n" );
    fprintf( out, "                         library and subdirectory targets.  All other Makefiles\n" );
    fprintf( out, "                         are created as wrappers, which will in turn run qmake.\n" );
    fprintf( out, "\n" );
    fprintf( out, " *  -no-fast ........... Configure Qt normally by generating Makefiles for all \n" );
    fprintf( out, "                         project files.                                        \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, "    -largefile ......... Use largefile support (64bit).                        \n" );
    fprintf( out, " *  -no-largefile ...... Don't use largefile support.                          \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, " *  -thread ............ Compile with thread support.                          \n" );
    fprintf( out, "    -no-thread ......... Don't compile with thread support                     \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, " *  -no-gif ............ Do not compile in GIF reading support.                \n" );
    fprintf( out, "    -gif    ............ Compile in GIF reading support.                       \n" );
    fprintf( out, "    See src/kernel/qgif.h                                                      \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, "    -no-zlib ........... Do not compile in ZLIB support.                       \n" );
    fprintf( out, " *  -zlib    ........... Compile in ZLIB support.                              \n" );
    // fprintf(out," *  -qt-zlib ........... Use the zlib bundled with Qt.                         \n");
    // fprintf(out,"    -system-zlib ....... Use zlib from the operating system.                   \n");
    // fprintf(out,"    See http://www.info-zip.org/pub/infozip/zlib                             \n");
    fprintf( out, "\n" );
    fprintf( out, "    -platform target ... The operating system and compiler you are building on.\n" );
    fprintf( out, "    See the PLATFORMS file for a list of supported                           \n" );
    fprintf( out, "    operating systems and compilers.                                         \n" );
    fprintf( out, "                                                                               \n" );
    // fprintf(out,"    -Dstring ........... Add an explicit define to the preprocessor.           \n");
    // fprintf(out,"    -Istring ........... Add an explicit include path.                         \n");
    // fprintf(out,"    -Lstring ........... Add an explicit library path.                         \n");
    // fprintf(out,"    -Rstring ........... Add an explicit dynamic library runtime search path.  \n");
    // fprintf(out,"    -lstring ........... Add an explicit library.                              \n");
    fprintf( out, "                                                                               \n" );
    fprintf( out, "    -disable-<module> .. Disable <module> where module is one of:           \n" );
    fprintf( out, "                            [ $CFG_MODULES_AVAILABLE ]                         \n" );
    fprintf( out, "                                                                               \n" );
    if ( ifConfig( "sql" ) ) {
        fprintf( out, "    -qt-sql-<driver> ... Enable a SQL <driver> in the Qt Library, by default \n" );
        fprintf( out, "                            none are turned on.                              \n" );
        fprintf( out, "    -plugin-sql-<driver> Enable SQL <driver> as a plugin to be linked to     \n" );
        fprintf( out, "                            at run time.                                     \n" );
        fprintf( out, "    -no-sql-<driver> ... Disable SQL <driver> entirely.                      \n" );
        fprintf( out, "                                                                             \n" );
        fprintf( out, "                            Possible values for <driver>:                    \n" );
        fprintf( out, "                            [ %s ]\n", options.sqlDriverAvailable.join( " " ).latin1() );
        fprintf( out, "                                                                             \n" );
        fprintf( out, "                         By default none of the SQL drivers are built,       \n" );
        fprintf( out, "                         even if they are auto-detected.                     \n" );
        fprintf( out, "                                                                             \n" );
    }
    fprintf( out, "    -qt-style-<style> .. Enable a GUI <style> in the Qt Library, by default \n" );
    fprintf( out, "                            all available styles are on.                       \n" );
    fprintf( out, "    -plugin-style-<style>Enable GUI <style> as a plugin to be linked to     \n" );
    fprintf( out, "                            at run time.                                       \n" );
    fprintf( out, "    -no-style-<style> .. Disable GUI <style> entirely.                      \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, "                            Possible values for <style>:                       \n" );
    fprintf( out, "                            [ %s ]                                             \n", options.QStyles.join( " " ).latin1() );
    fprintf( out, "                                                                               \n" );
    fprintf( out, "    -no-pch ............ Do not use precompiled header support.             \n" );
    fprintf( out, " *  -pch ............... Use precompiled header support.                    \n" );
    fprintf( out, "                                                                            \n" );
    fprintf( out, "    -no-stl ............ Do not compile STL support.                        \n" );
    fprintf( out, " *  -stl ............... Compile STL support.                               \n" );
    fprintf( out, "                                                                            \n" );
    fprintf( out, "    -no-rtti ........... Do not compile RTTI support.                       \n" );
    fprintf( out, " *  -rtti .............. Compile RTTI support.                              \n" );
    fprintf( out, "                                                                            \n" );
    fprintf( out, "    -no-exceptions ..... Enable exceptions on platforms that support it.    \n" );
    fprintf( out, "    -exceptions ........ Disable exceptions on platforms that support it.   \n" );
    fprintf( out, "                                                                            \n" );
    fprintf( out, "    -verbose ........... Print verbose information about each step of the   \n" );
    fprintf( out, "    -v ................. configure process.                                 \n" );
    fprintf( out, "                                                                            \n" );
    fprintf( out, "    -rebuild............ rebuild makefiles using the current configuration. \n" );
    fprintf( out, "                                                                            \n" );
    fprintf( out, "    -dryrun ............ don't execute real command only display them.      \n" );
    fprintf( out, "                                                                            \n" );
    fprintf( out, "    -no-dsp ............ Do not create Microsoft Visual C++ files.          \n" );
    fprintf( out, " *  -dsp    ............ Create Microsoft Visual C++ dsp files.             \n" );
    fprintf( out, "                                                                            \n" );
    fprintf( out, "    -make-files ........ create makefiles for a outside qt source package.  \n" );
    fprintf( out, "                                                                            \n" );
    // fprintf(out,"    -continue........... Continue as far as possible if an error occurs.    \n");
    // fprintf(out,"                                                                               \n");
    fprintf( out, "    -help .............. Print help and usage information.                  \n" );
    fprintf( out, "    -h ................. Print help and usage information.                  \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, "Image Formats:                                                                 \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, "    -plugin-imgfmt-<fmt> Enable <format> to be linked to at runtime.           \n" );
    fprintf( out, "                            Uses specified lib<format>.                        \n" );
    fprintf( out, "    -qt-imgfmt-<format>  Enable <format> to be linked into Qt.                 \n" );
    fprintf( out, "                            Uses specified lib<format>.                        \n" );
    fprintf( out, "    -no-imgfmt-<format>  Fully disable <format> from Qt.                        \n" );
    fprintf( out, "                                                                               \n" );
    fprintf( out, "                            Possible values for <format>:                      \n" );
    fprintf( out, "                            [ png jpeg mng ]                                   \n" );
    fprintf( out, "                                                                               \n" );
    // fprintf(out,"Third Party Image Loading Libraries:                                           \n");
    // fprintf(out,"                                                                               \n");
    // fprintf(out," *  -qt-libpng ......... Use the libpng bundled with Qt.                       \n");
    // fprintf(out,"    -system-libpng ..... Use libpng from the operating system.                 \n");
    // fprintf(out,"    See http://www.libpng.org/pub/png                                        \n");
    // fprintf(out,"                                                                               \n");
    // fprintf(out," *  -qt-libjpeg ........ Use the libjpeg bundled with Qt.                      \n");
    // fprintf(out,"    -system-libjpeg .... Use jpeglib from the operating system.                \n");
    // fprintf(out,"    See http://www.ijg.org                                                   \n");
    // fprintf(out,"                                                                               \n");
    // fprintf(out," *  -qt-libmng ......... Use the libmng bundled with Qt.                       \n");
    // fprintf(out,"    -system-libmng ..... Use libmng from the operating system.                 \n");
    // fprintf(out,"    See http://www.libmng.com                                                \n");
}


int main( int argc, char **argv )
{
    QStringList commandLine;
    for ( int i = 0; i < argc; ++i ) {
        commandLine.append( argv[ i ] );
    }

    parseCommandLine( commandLine );

    if ( options.logfile ) {
        out = fopen( options.logfile, "w+" );
        if ( !out ) {
            qWarning( "Error, could not open log file" );
            out = stdout;
        }
    }

    if ( options.rebuild )
        doBuild();
    else if ( options.makeFiles )
        makeFiles();
    else
        doConfigure();

    if ( out != stdout )
        fclose( out );

    return 0;
}
