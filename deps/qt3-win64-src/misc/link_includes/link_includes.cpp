/****************************************************************************
**
** Copyright (C) 2004 Qt3/Win32.  All rights reserved.
**
** This file is part of Qt3/Win32 from kde-cygwin.sorceforge.net
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <qdir.h>
#include <qregexp.h>
#include "qtmd5.h"

//#define DEBUG_LINK_INCLUDES

bool verbose = false;
bool dryrun = false;
bool onlyNew = true;

void print_help( const QString &fn )
{
    fprintf( stdout, "%s for Qt3/win32 (GPL-Version)\n", fn.ascii() );
    fprintf( stdout, "Usage: -verbose : be verbose\n" );
    fprintf( stdout, "       -dryrun  : don't copy files\n" );
    fprintf( stdout, "       -all     : copy all files although src and dst are the same\n" );
    exit( 0 );
}

void check_dir( const QString &path )
{
    QDir dir( path );
    if ( !dir.exists() ) {
        fprintf( stderr, "%s not found!", path.latin1() );
        exit( 1 );
    }
}

void removeHeader( const QString &path , const QString &filter , const QStringList &noRemove )
{
    QDir d ( path );
    d.setFilter( QDir::Files | QDir::Hidden | QDir::NoSymLinks );
    d.setNameFilter( filter );
    d.setSorting( QDir::Name );

    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    for ( ; ( fi = it.current() ) != 0; ++it ) {
        if ( noRemove.contains( fi->fileName() ) ) {
            if ( verbose )
                fprintf( stdout, "skipping %s\n", fi->fileName().latin1() );
            continue;
        }
        if ( verbose )
            fprintf( stdout, "removing %s\n", fi->fileName().latin1() );
        if ( dryrun )
            continue;
        d.remove( fi->fileName() );
    }
}

void copyFile( const QString& src, const QString& dest )
{
#ifdef DEBUG_LINK_INCLUDES
    fprintf( stdout, "copyFile( %s, %s )\n", src.latin1(), dest.latin1() );
#endif

    QFile s( src );
    QFile d( dest );
    if ( onlyNew ) {
        QFileInfo src_info( src );
        QFileInfo dst_info( dest );
        if ( ( dst_info.exists() ) && ( src_info.size() == dst_info.size() ) ) {
            s.open( IO_ReadOnly | IO_Translate );
            d.open( IO_ReadOnly | IO_Translate );
            QByteArray s_ba = s.readAll();
            if ( qtMD5( s_ba ) == qtMD5( d.readAll() ) ) {
                s.close();
                d.close();
                if ( verbose )
                    fprintf( stdout, "File %s isn't changed - skipping\n",
                             QDir::convertSeparators( src ).ascii() );
                return ;
            }
            if ( dryrun )
                return ;
            s.close();
            d.close();
            d.open( IO_WriteOnly | IO_Truncate | IO_Translate );
            d.writeBlock( s_ba );
            d.close();
            return ;
        }
    }
    if ( verbose )
        fprintf( stdout, "copying %s to %s\n", QDir::convertSeparators( src ).ascii(),
                 QDir::convertSeparators( dest ).ascii() );
    if ( dryrun )
        return ;


    s.open( IO_ReadOnly | IO_Translate );
    d.open( IO_WriteOnly | IO_Truncate | IO_Translate );
    d.writeBlock( s.readAll() );
    s.close();
    d.close();
}

void copyHeader( const QString &srcdir, QString &destdir, const QString &filter = "q*.h" )
{
#ifdef DEBUG_LINK_INCLUDES
    fprintf( stdout, "copyHeader( %s, %s )\n", srcdir.latin1(), destdir.latin1() );
#endif

    QDir d ( srcdir );
    d.setFilter( QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoSymLinks );
    d.setNameFilter( filter );
    d.setSorting( QDir::Size | QDir::Reversed );
    d.setMatchAllDirs( true );

    const QFileInfoList *list = d.entryInfoList();
    QFileInfoListIterator it( *list );
    QFileInfo *fi;

    for ( ; ( fi = it.current() ) != 0; ++it ) {
        // skip "."  and ".." dir
        if ( fi->isDir() && ( fi->fileName() == "." || fi->fileName() == ".." ) )
            continue;

#ifdef DEBUG_LINK_INCLUDES

        fprintf( stdout, "fi->isDir(): %d (%s)\n", fi->isDir(), fi->fileName().latin1() );
#endif

        if ( fi->isDir() ) {
            copyHeader( d.absPath() + "/" + fi->fileName(), destdir );
        } else if ( fi->fileName().endsWith( "_p.h" ) )
            copyFile( fi->absFilePath(), destdir + "/private/" + fi->fileName() );
        else
            copyFile( fi->absFilePath(), destdir + "/" + fi->fileName() );
    }
}

int main( int argc, char **argv )
{
    for ( int i = 1; i < argc; ++i ) {
        QString arg( argv[ i ] );
        arg.remove( QRegExp( "^(-)*" ) );
        if ( arg == "verbose" )
            verbose = true;
        else if ( arg == "dryrun" )
            dryrun = true;
        else if ( arg == "all" )
            onlyNew = false;
        else if ( arg == "help" )
            print_help( argv[ 0 ] );
        else {
            fprintf( stderr, "Unknown parameter %s\n", argv[ i ] );
            print_help( argv[ 0 ] );
            exit( 1 );
        }
    }
    QString root_prefix = getenv( "QTDIR" );
    QString include_prefix = QString( root_prefix + "\\include" );

    check_dir( root_prefix + "\\include" );
    check_dir( root_prefix + "\\include\\private" );
    check_dir( root_prefix + "\\src" );
    check_dir( root_prefix + "\\extensions" );
    check_dir( root_prefix + "\\tools\\designer\\uilib" );
    check_dir( root_prefix + "\\tools\\assistant\\lib" );

    QStringList noRemove;
    noRemove << "qconfig.h" << "qglobal.h" << "qmodules.h" << "qnamespace.h";

    if ( !onlyNew ) {
        removeHeader( root_prefix + "\\include", "q*.h" , noRemove );
        removeHeader( root_prefix + "\\include\\private", "q*.h" , noRemove );
    }
    copyHeader( root_prefix + "\\src", include_prefix );
    copyHeader( root_prefix + "\\extensions", include_prefix );
    copyHeader( root_prefix + "\\tools\\designer\\uilib", include_prefix );
    copyHeader( root_prefix + "\\tools\\assistant\\lib", include_prefix );
    copyHeader( root_prefix + "\\extensions\\nsplugin\\src", include_prefix, "j*.h" );
    copyHeader( root_prefix + "\\extensions\\nsplugin\\src", include_prefix, "np*.h" );

    return 0;
}
