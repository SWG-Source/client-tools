/****************************************************************************
** $Id: qfileinfo_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QFileInfo class
**
** Created : 20030709
**
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2004 Ralf Habacker
** Copyright (C) 2004 Christian Ehrlicher
** Copyright (C) 2004 Peter Kuemmel
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

#include "qt_windows.h"
#include "qfileinfo.h"
#include "qdatetime.h"
#include "qdir.h"
#include "qfiledefs_p.h"
#include "qregexp.h"
#include <windows.h>
#include <shlobj.h>

/* msvc 6.0 doesn't know about this */
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES	((DWORD)-1)
#endif

Q_EXPORT int qt_ntfs_permission_lookup = 0;


void QFileInfo::slashify( QString& data )
{
    // now it really works - Perl is simply better ;)
    data = data.replace( QRegExp( "\\\\" ), "/" );
    if ( data.at( data.length() - 1 ) == '/' && data.length() > 3 )
        data.truncate( data.length() - 1 );
}


void QFileInfo::makeAbs( QString & )
{
    return ;
}

/*!
    Returns TRUE if this object points to a file. Returns FALSE if the
    object points to something which isn't a file, e.g. a directory or
    a symlink.

    \sa isDir(), isSymLink()
*/
bool QFileInfo::isFile() const
{
    if ( !fic || !cache )
        doStat();
    return fic ? ( fic->st.st_mode & S_IFMT ) == S_IFREG : FALSE;
}

/*!
    Returns TRUE if this object points to a directory or to a symbolic
    link to a directory; otherwise returns FALSE.

    \sa isFile(), isSymLink()
*/
bool QFileInfo::isDir() const
{
    if ( !fic || !cache )
        doStat();
    return fic ? ( fic->st.st_mode & S_IFMT ) == S_IFDIR : FALSE;
}

/*!
    Returns TRUE if this object points to a symbolic link (or to a
    shortcut on Windows); otherwise returns FALSE.

    \sa isFile(), isDir(), readLink()
*/

bool QFileInfo::isSymLink() const
{
    return ( fn.right( 4 ) == ".lnk" );
}

/*!
  Returns TRUE if the file is hidden; otherwise returns FALSE.

  On Unix-like operating systems, including Mac OS X, a file is hidden if its
  name begins with ".". On Windows a file is hidden if its hidden attribute is
  set.
*/
bool QFileInfo::isHidden() const
{
    QString tmp = fn;
    tmp = tmp.replace( '/', '\\' );
    DWORD attrs = QT_WA_INLINE( GetFileAttributesW( ( LPCWSTR ) tmp.ucs2() ),
                                GetFileAttributesA( QFile::encodeName( tmp ) ) );
    if ( attrs == INVALID_FILE_ATTRIBUTES) {
        int err = GetLastError();
        if ( err == ERROR_ACCESS_DENIED )
           return true;
        return false;
    }
    return ( attrs & FILE_ATTRIBUTE_HIDDEN );
}

/*!
  Returns the name a symlink (or shortcut on Windows) points to, or a
  null QString if the object isn't a symbolic link.

  This name may not represent an existing file; it is only a string.
  QFileInfo::exists() returns TRUE if the symlink points to an
  existing file.

  \sa exists(), isSymLink(), isDir(), isFile()
*/

QString QFileInfo::readLink() const
{
    QString ret;
// avoid the need for ole32.lib when creating qmake, moc & friends
#ifdef QT_MAKEDLL
    QT_WA({
        bool neededCoInit = false;
        IShellLink *psl;                            // pointer to IShellLink i/f
        HRESULT hres;
        WIN32_FIND_DATA wfd;
        TCHAR szGotPath[MAX_PATH];
        // Get pointer to the IShellLink interface.
        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                    IID_IShellLink, (LPVOID *)&psl);

        if(hres == CO_E_NOTINITIALIZED) { // COM was not initalized
            neededCoInit = true;
            CoInitialize(NULL);
            hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                        IID_IShellLink, (LPVOID *)&psl);
        }
        if(SUCCEEDED(hres)) {    // Get pointer to the IPersistFile interface.
            IPersistFile *ppf;
            hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
            if(SUCCEEDED(hres))  {
                hres = ppf->Load((LPOLESTR)fn.ucs2(), STGM_READ);
                if(SUCCEEDED(hres)) {        // Resolve the link.

                    hres = psl->Resolve(0, SLR_ANY_MATCH | SLR_NO_UI | SLR_UPDATE);

                    if(SUCCEEDED(hres)) {
                        memcpy(szGotPath, (TCHAR*)fn.ucs2(), (fn.length()+1)*sizeof(QChar));
                        hres = psl->GetPath(szGotPath, MAX_PATH, &wfd, SLGP_UNCPRIORITY);
                        ret = QString::fromUcs2((ushort*)szGotPath);
                    }
                }
                ppf->Release();
            }
            psl->Release();
        }
        if(neededCoInit)
            CoUninitialize();
    } , {
	    bool neededCoInit = false;
        IShellLinkA *psl;                            // pointer to IShellLink i/f
        HRESULT hres;
        WIN32_FIND_DATAA wfd;
        char szGotPath[MAX_PATH];
        // Get pointer to the IShellLink interface.

        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                    IID_IShellLinkA, (LPVOID *)&psl);

        if(hres == CO_E_NOTINITIALIZED) { // COM was not initalized
            neededCoInit = true;
            CoInitialize(NULL);
            hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                        IID_IShellLinkA, (LPVOID *)&psl);
        }
        if(SUCCEEDED(hres)) {    // Get pointer to the IPersistFile interface.
            IPersistFile *ppf;
            hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
            if(SUCCEEDED(hres))  {
                hres = ppf->Load((LPOLESTR)QFileInfo(fn).absFilePath().ucs2(), STGM_READ);
                if(SUCCEEDED(hres)) {        // Resolve the link.

                    hres = psl->Resolve(0, SLR_ANY_MATCH);

                    if(SUCCEEDED(hres)) {
                        QByteArray lfn = fn.local8Bit();
                        memcpy(szGotPath, lfn.data(), (lfn.size()+1)*sizeof(char));
                        hres = psl->GetPath((char*)szGotPath, MAX_PATH, &wfd, SLGP_UNCPRIORITY);
                        ret = QString::fromLocal8Bit(szGotPath);

                    }
                }
                ppf->Release();
            }
            psl->Release();
        }
        if(neededCoInit)
            CoUninitialize();
    });
#endif
    return ret;
}

/*!
  Returns the id of the owner of the file.

  On Windows and on systems where files do not have owners this function
  returns ((uint) -2).

  \sa owner(), group(), groupId()
*/

uint QFileInfo::ownerId() const
{
    static const uint nobodyID = (uint) -2;
    return nobodyID;
}

/*!
  Returns the owner of the file. On Windows, on systems where files do
  not have owners, or if an error occurs, a null string is returned.

  This function can be time consuming under Unix (in the order of
  milliseconds).

  \sa ownerId(), group(), groupId()
*/

QString QFileInfo::owner() const
{
    return QString::null;
}

/*!
  Returns the group of the file. On Windows, on systems where files do
  not have groups, or if an error occurs, a null string is returned.

  This function can be time consuming under Unix (in the order of
  milliseconds).

  \sa groupId(), owner(), ownerId()
*/

QString QFileInfo::group() const
{
    return QString::null;
}

/*!
  Returns the id of the group the file belongs to.

  On Windows and on systems where files do not have groups this function
  always returns (uint) -2.

  \sa group(), owner(), ownerId()
*/

uint QFileInfo::groupId() const
{
    static const uint nogroupID = (uint) -2;
    return nogroupID;
}


/*!
  \fn bool QFileInfo::permission( int permissionSpec ) const

  Tests for file permissions.  The \a permissionSpec argument can be several
  flags of type PermissionSpec OR-ed together to check for permission
  combinations.

  On systems where files do not have permissions this function always
  returns TRUE.

  Example:
  \code
    QFileInfo fi( "/tmp/archive.tar.gz" );
    if ( fi.permission( QFileInfo::WriteUser | QFileInfo::ReadGroup ) )
        qWarning( "I can change the file; my group can read the file.");
    if ( fi.permission( QFileInfo::WriteGroup | QFileInfo::WriteOther ) )
        qWarning( "The group or others can change the file!" );
  \endcode

  \sa isReadable(), isWritable(), isExecutable()
*/

bool QFileInfo::permission( int /*permissionSpec*/ ) const
{
    return TRUE;
}

void QFileInfo::doStat() const
{
    if ( fn.isEmpty() )
        return ; // Fix crash in qmake
    QFileInfo *that = ( ( QFileInfo* ) this );   // mutable function
    if ( !that->fic )
        that->fic = new QFileInfoCache;
    QT_STATBUF *b = &that->fic->st;

    QString file = fn;
    file = file.replace( '/', '\\' );
#ifdef QT_LARGEFILE_SUPPORT
    if ( QT_WA_INLINE( _wstati64( (wchar_t*) file.ucs2(), b ),
                       _stati64( QFile::encodeName( file ), b ) ) == -1 ) {
#else
    if ( QT_WA_INLINE( _wstat( (wchar_t*) file.ucs2(), b ),
                       _stat( QFile::encodeName( file ), b ) ) == -1 ) {
#endif
        delete that->fic;
        that->fic = 0;
    }
}

/*!
  Returns the directory path of the file.

  If \a absPath is TRUE an absolute path is returned.

  \sa dir(), filePath(), fileName(), isRelative()
*/
#ifndef QT_NO_DIR
QString QFileInfo::dirPath( bool absPath ) const
{
    QString s;
    if ( absPath )
        s = absFilePath();
    else
        s = fn;
    int pos = s.findRev( '/' );
    if ( pos == -1 ) {
        return QString::fromLatin1( "." );
    } else {
        if ( pos == 0 )
            return QDir::rootDirPath();
		if ( pos == 2 && s[ 1 ] == ':' )
			return s.left( pos + 1 );
        return s.left( pos );
    }
}
#endif

/*!
  Returns the name of the file, the file path is not included.

  Example:
  \code
     QFileInfo fi( "/tmp/archive.tar.gz" );
     QString name = fi.fileName();                // name = "archive.tar.gz"
  \endcode

  \sa isRelative(), filePath(), baseName(), extension()
*/

QString QFileInfo::fileName() const
{
    int p = fn.findRev( '/' );
    if ( p == -1 ) {
        return fn;
    } else {
        return fn.mid( p + 1 );
    }
}
