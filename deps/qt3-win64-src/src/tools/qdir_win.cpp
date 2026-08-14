/****************************************************************************
** $Id: qdir_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QDir class
**
** Created : 20030709
**
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2004 Ralf Habacker
** Copyright (C) 2004 Christian Ehrlicher
** Copyright (C) 2004 Christopher January
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact kde-cygwin@kde.org if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qplatformdefs.h"
#include "qdir.h"

#ifndef QT_NO_DIR

#include "qt_windows.h"
#include "qdir_p.h"
#include "qfileinfo.h"
#include "qregexp.h"
#include "qstringlist.h"

#ifdef QT_THREAD_SUPPORT
#include "qmutex.h"
#include "qmutexpool_p.h"
#endif // QT_THREAD_SUPPORT

static QString p_getenv( QString name )
{
    DWORD len = QT_WA_INLINE(
                    GetEnvironmentVariableW ( ( LPCWSTR ) qt_winTchar ( name, true ), NULL, 0 ),
                    GetEnvironmentVariableA ( ( LPCSTR ) name.data(), NULL, 0 ) );
    if ( len == 0 )
        return QString::null;
    /* ansi: we allocate too much memory, but this shouldn't be the problem here ... */
    LPTSTR buf = new WCHAR[ len ];
    len = QT_WA_INLINE(
              GetEnvironmentVariableW ( ( LPCWSTR ) qt_winTchar ( name, true ), buf, len ),
              GetEnvironmentVariableA ( ( LPCSTR ) name.data(), ( char* ) buf, len ) );
    if ( len == 0 )
        return QString::null;
    QString ret = qt_winQString ( buf );
    delete[] buf;
    return ret;
}

void QDir::slashify( QString& data )
{
    data = data.replace( QRegExp( "\\\\" ), "/" );
    /* CE:
       We can't add c:/ here since we would break cleanDirPath() and maybe others
       don't know which functions expects that c:/ is added in front -> this functions
       need to do this be their own
    if ( data[ 0 ] == '/' )
        data = rootDirPath().left( 2 ) + data; */
}

QString QDir::homeDirPath()
{
    QString d = p_getenv ( "HOME" );
    if ( d.isNull () ) {
        d = p_getenv ( "USERPROFILE" );
        if ( d.isNull () ) {
            QString homeDrive = p_getenv ( "HOMEDRIVE" );
            QString homePath = p_getenv ( "HOMEPATH" );
            if ( !homeDrive.isNull () && !homePath.isNull () ) {
                d = homeDrive + homePath;
            } else {
                d = rootDirPath ();
            }
        }
    }
    slashify( d );
    return d;
}

QString QDir::canonicalPath() const
{
    QString r;

    char cur[ PATH_MAX ];
    char tmp[ PATH_MAX ];
    QT_GETCWD( cur, PATH_MAX );
    if ( QT_CHDIR( QFile::encodeName( dPath ) ) >= 0 ) {
        QT_GETCWD( tmp, PATH_MAX );
        r = QFile::decodeName( tmp );
    }
    QT_CHDIR( cur );

    slashify( r );
    return r;
}

bool QDir::mkdir( const QString &dirName, bool acceptAbsPath ) const
{
    QT_WA(
        return _wmkdir( ( LPCWSTR ) filePath( dirName, acceptAbsPath ).ucs2() ) == 0;
        ,
        return _mkdir( QFile::encodeName( filePath( dirName, acceptAbsPath ) ) ) == 0;
    );
}

bool QDir::rmdir( const QString &dirName, bool acceptAbsPath ) const
{
    QT_WA(
        return _wrmdir( ( LPCWSTR ) filePath( dirName, acceptAbsPath ).ucs2() ) == 0;
        ,
        return _rmdir( QFile::encodeName( filePath( dirName, acceptAbsPath ) ) ) == 0;
    );
}

bool QDir::isReadable() const
{
    QString path = dPath;
    if ( ( path[ 0 ] == '\\' ) || ( path[ 0 ] == '/' ) )
        path = rootDirPath() + path;
    return QT_ACCESS( QFile::encodeName( path ), R_OK ) == 0;
}

bool QDir::isRoot() const
{
    QString path = dPath;
    slashify( path );
    return path == rootDirPath ();
}

bool QDir::rename( const QString &name, const QString &newName,
                   bool acceptAbsPaths )
{
    if ( name.isEmpty() || newName.isEmpty() ) {
#if defined(QT_CHECK_NULL)
        qWarning( "QDir::rename: Empty or null file name(s)" );
#endif

        return FALSE;
    }
    QString fn1 = filePath( name, acceptAbsPaths );
    QString fn2 = filePath( newName, acceptAbsPaths );
    QT_WA(
        return MoveFileW( ( LPCWSTR ) fn1.ucs2(), ( LPCWSTR ) fn2.ucs2() ) != 0;
        ,
        return MoveFileA( QFile::encodeName( fn1 ), QFile::encodeName( fn2 ) ) != 0;
    );
}

bool QDir::setCurrent( const QString &path )
{
    if ( !QDir( path ).exists() )
        return false;

    QT_WA(
        return ( SetCurrentDirectoryW( ( LPCWSTR ) path.ucs2() ) >= 0 );
        ,
        return ( SetCurrentDirectoryA( QFile::encodeName( path ) ) >= 0 );
    );
}

QString QDir::currentDirPath()
{
    QString ret;
    QT_WA( {
               DWORD size = 0;
               WCHAR currentName[ PATH_MAX ];
               size = ::GetCurrentDirectoryW( PATH_MAX, currentName );
               if ( size != 0 ) {
               if ( size > PATH_MAX ) {
                       WCHAR * newCurrentName = new WCHAR[ size ];
                       if ( ::GetCurrentDirectoryW( PATH_MAX, newCurrentName ) != 0 )
                           ret = QString::fromUcs2( ( ushort* ) newCurrentName );
                       delete [] newCurrentName;
                   } else {
                       ret = QString::fromUcs2( ( ushort* ) currentName );
                   }
               }
           } , {
               DWORD size = 0;
               char currentName[ PATH_MAX ];
               size = ::GetCurrentDirectoryA( PATH_MAX, currentName );
               if ( size != 0 )
               ret = QString::fromLocal8Bit( currentName );
           } )
        ;
    if ( ret.length() >= 2 && ret[ 1 ] == ':' )
        ret[ 0 ] = ret.at( 0 ).upper(); // Force uppercase drive letters.
    slashify( ret );
    return ret;
}

QString QDir::rootDirPath()
{
    QString d = p_getenv ( "SystemDrive" );
    if ( d.isNull () )
        d = QString::fromLatin1( "c:" );  // not "c:\\" !
    slashify ( d );
    return d;
}

bool QDir::isRelativePath( const QString &path )
{
    if ( path.isEmpty() )
        return TRUE;
    int p = 0;
    if ( path[ 0 ].isLetter() && path[ 1 ] == ':' )
        p = 2; // we have checked the first 2.
    return ( ( path[ p ] != '/' ) && ( path[ p ] != '\\' ) );
}

#undef IS_SUBDIR
#undef IS_RDONLY
#undef IS_ARCH
#undef IS_HIDDEN
#undef IS_SYSTEM
#undef FF_ERROR

#define IS_SUBDIR   FILE_ATTRIBUTE_DIRECTORY
#define IS_RDONLY   FILE_ATTRIBUTE_READONLY
#define IS_ARCH     FILE_ATTRIBUTE_ARCHIVE
#define IS_HIDDEN   FILE_ATTRIBUTE_HIDDEN
#define IS_SYSTEM   FILE_ATTRIBUTE_SYSTEM
bool QDir::readDirEntries( const QString &nameFilter,
                           int filterSpec, int sortSpec )
{
    if ( !fList ) {
        fList = new QStringList;
        Q_CHECK_PTR( fList );
    }
    if ( !fiList ) {
        fiList = new QFileInfoList;
        Q_CHECK_PTR( fiList );
        fiList->setAutoDelete( TRUE );
    } else {
        fList->clear();
        fiList->clear();
    }

    QValueList<QRegExp> filters = qt_makeFilterList( nameFilter );

    bool doDirs     = ( ( filterSpec & QDir::Dirs ) != 0 || allDirs );
    bool doFiles    = ( filterSpec & QDir::Files ) != 0;
    bool noSymLinks = ( filterSpec & QDir::NoSymLinks ) != 0;
    bool doReadable = ( filterSpec & QDir::Readable ) != 0;
    bool doWritable = ( filterSpec & QDir::Writable ) != 0;
    bool doExecable = ( filterSpec & QDir::Executable ) != 0;
    bool doModified = ( filterSpec & QDir::Modified ) != 0;
    bool doHidden   = ( filterSpec & QDir::Hidden ) != 0;
    bool doSystem   = ( filterSpec & QDir::System ) != 0;
    // show hidden files if the user asks explicitly for e.g. .*
    if ( !doHidden && !nameFilter.length() ) {
        if ( nameFilter[ 0 ] == '.' ) {
            doHidden = true;
        }
    }

    HANDLE handle;
    WIN32_FIND_DATAW finfo;

    QString winpath = dPath.replace( "/", "\\" );
    QString path;

    // work around a bug whel an url was given but be aware of unc-paths !
    if ( ( winpath[ 0 ] == '\\' ) && ( winpath[ 1 ] != '\\' ) )
        winpath = rootDirPath() + winpath;

    if ( winpath.isEmpty() ) {
#if defined(CHECK_NULL)
        qWarning( "QDir::readDirEntries: Cannot convert the path: %s",
                  QFile::encodeName( dPath ).data() );
#endif

        return FALSE;
    }

    if ( winpath.right( 1 ) != "\\" )
        winpath += "\\";
    path = winpath;
    winpath += "*.*";
    // from qt4: Cast is safe, since char is at end of WIN32_FIND_DATA
    handle = QT_WA_INLINE( FindFirstFileW ( ( LPCWSTR ) winpath.ucs2(), &finfo ),
                           FindFirstFileA ( QFile::encodeName( winpath ), ( WIN32_FIND_DATAA* ) & finfo ) );
    DWORD lasterr = GetLastError ();
    if ( handle == INVALID_HANDLE_VALUE && ( lasterr != ERROR_NO_MORE_FILES ) ) {
#if defined(CHECK_NULL)
        qWarning( "QDir::readDirEntries: Cannot read the directory: %s",
                  QFile::encodeName( dPath ).local8Bit() );
#endif

        return FALSE;
    }
    do {
        QString fn = QT_WA_INLINE ( QString::fromUcs2( ( const unsigned short* ) finfo.cFileName ),
                                    QFile::decodeName( ( const char * ) finfo.cFileName ) );
        if ( finfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY ) {
            /* remove .lnk suffix in case of symlinks */
            fn.remove( ".lnk$" );
        }
        int attrib = finfo.dwFileAttributes;

        bool isDir      = ( attrib & IS_SUBDIR ) != 0;
        bool isFile     = !isDir;
        bool isSymLink  = false;
        bool isReadable = true;
        bool isWritable = ( attrib & IS_RDONLY ) == 0;
        bool isExecable = false;
        bool isModified = ( attrib & IS_ARCH ) != 0;
        bool isHidden   = ( attrib & IS_HIDDEN ) != 0;
        bool isSystem   = ( attrib & IS_SYSTEM ) != 0;

        if ( !qt_matchFilterList( filters, fn ) && !( allDirs && isDir ) )
            continue;
        if ( ( doDirs && isDir ) || ( doFiles && isFile ) ) {
            if ( doExecable ) {
                QString ext = fn.right( 4 ).lower();
                if ( ext == ".exe" || ext == ".com" || ext == ".bat" ||
                        ext == ".pif" || ext == ".cmd" || isDir )
                    isExecable = true;
            }
            if ( noSymLinks && isSymLink )
                continue;
            if ( ( filterSpec & RWEMask ) != 0 )
                if ( ( doReadable && !isReadable ) ||
                        ( doWritable && !isWritable ) ||
                        ( doExecable && !isExecable ) )
                    continue;
            if ( doModified && !isModified )
                continue;
            if ( !doHidden && isHidden )
                continue;
            if ( !doSystem && isSystem )
                continue;
            fiList->append( new QFileInfo( path + fn ) );
        }
    } while ( QT_WA_INLINE( FindNextFileW ( handle, &finfo ),
                            FindNextFileA ( handle, ( WIN32_FIND_DATAA* ) & finfo ) ) );

    lasterr = GetLastError ();
    FindClose ( handle );
    if ( lasterr != ERROR_NO_MORE_FILES ) {
#if defined(CHECK_NULL)
        qWarning( "QDir::readDirEntries: Cannot close the directory: %s",
                  dPath.local8Bit().data() );
#endif

    }

    // Sort...
    int i;
    if ( fiList->count() ) {
        QDirSortItem * si = new QDirSortItem[ fiList->count() ];
        QFileInfo* itm;
        i = 0;
        for ( itm = fiList->first(); itm; itm = fiList->next() )
            si[ i++ ].item = itm;
        qt_cmp_si_sortSpec = sortSpec;
        qsort( si, i, sizeof( si[ 0 ] ), qt_cmp_si );
        // put them back in the list
        fiList->setAutoDelete( FALSE );
        fiList->clear();
        int j;
        for ( j = 0; j < i; j++ ) {
            fiList->append( si[ j ].item );
            fList->append( si[ j ].item->fileName() );
        }
        delete [] si;
        fiList->setAutoDelete( TRUE );
    }

    if ( filterSpec == ( FilterSpec ) filtS && sortSpec == ( SortSpec ) sortS &&
            nameFilter == nameFilt )
        dirty = FALSE;
    else
        dirty = TRUE;
    return TRUE;
}
#undef        IS_SUBDIR
#undef        IS_RDONLY
#undef        IS_ARCH
#undef        IS_HIDDEN
#undef        IS_SYSTEM
#undef        FF_ERROR

// at most one instance of QFileInfoList is leaked, and this variable
// points to that list
static QFileInfoList * knownMemoryLeak = 0;
const QFileInfoList * QDir::drives()
{
    if ( !knownMemoryLeak ) {

#ifdef QT_THREAD_SUPPORT
        QMutexLocker locker( qt_global_mutexpool ?
                             qt_global_mutexpool->get
                             ( &knownMemoryLeak ) : 0 );
#endif // QT_THREAD_SUPPORT

        if ( !knownMemoryLeak ) {
            knownMemoryLeak = new QFileInfoList;

            /* There maybe we should use GetLogicalDriveStrings next time ... */
            Q_UINT32 driveBits = ( Q_UINT32 ) GetLogicalDrives() & 0x3ffffff;
            char driveName[ 4 ];
            qstrcpy( driveName, "A:/" );
            while ( driveBits ) {
                if ( driveBits & 1 )
                    knownMemoryLeak->append( new QFileInfo( driveName ) );
                driveName[ 0 ] ++;
                driveBits = driveBits >> 1;
            }
        }
    }
    return knownMemoryLeak;
}

void qt_dir_cleanup()
{
    if ( knownMemoryLeak ) {
        for ( QFileInfo * fi = knownMemoryLeak->first(); fi; fi = knownMemoryLeak->next() )
            delete fi;
        delete knownMemoryLeak;
    }
}

#endif //QT_NO_DIR
