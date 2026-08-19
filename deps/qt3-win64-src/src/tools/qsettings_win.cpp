/****************************************************************************
** $Id: qsettings_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QSettingsPrivate class for Windows
**
** Created: 20040918
**
** Copyright (C) 2004 Christian Ehrlicher
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

#include "qsettings.h"
#ifndef QT_NO_SETTINGS

#include "qt_windows.h"
#include <winreg.h>

#include "qstring.h"
#include "qregexp.h"
#include "qsettings_p.h"

/* MSVC++ 6.0 doesn't define this :( */
#ifndef REG_QWORD
#define REG_QWORD_LITTLE_ENDIAN (11)
#define REG_QWORD (11)
#endif

bool qt_verify_key( const QString & ); //qsettings.cpp

class QSettingsSysPrivate
{
public:
    QSettingsSysPrivate();
    QStringList searchPaths;

    /* Helper functions */
    /* Opens a key, first tries in HKEY_CURRENT_USER/Software, then
       in HKEY_LOCAL_MACHINE/Software unless global == true (than only HKLM).
       If key doesn't exist, it is created --> for writing. */
    HKEY createKey( const QString &key, REGSAM access , QString &value, bool global );
    /* Opens a key, first tries in HKEY_CURRENT_USER/Software, then
       in HKEY_LOCAL_MACHINE/Software  unless global == true (than only HKLM).
       If key doesn't exist, open will fail --> for reading. */
    HKEY openKey( const QString &key, REGSAM access , QString &value, bool global );
    /* Search a path in the searchpaths and returns the first possible
       if none found -> return first path */
    void searchPath( HKEY &scope, QString &path, REGSAM access, bool global, bool readonly );
    /* Splits a key in main_key and value */
    void splitKey ( const QString &key, QString &main_key, QString &value );
};

QSettingsSysPrivate::QSettingsSysPrivate()
{}

HKEY QSettingsSysPrivate::createKey( const QString &key, REGSAM access, QString &value , bool global )
{
    HKEY res_key = NULL;
    HKEY scope;
    QString main_key;

    splitKey( key, main_key, value );
    searchPath( scope, main_key, access, global, 0 );

    QT_WA_INLINE(
        RegCreateKeyExW( scope, ( LPCTSTR ) main_key.ucs2(), 0, NULL,
                         REG_OPTION_NON_VOLATILE, access, NULL, &res_key , NULL ),
        RegCreateKeyExA( scope, ( LPCSTR ) main_key.latin1(), 0, NULL,
                         REG_OPTION_NON_VOLATILE, access, NULL, &res_key , NULL ) );

    return res_key;
}

HKEY QSettingsSysPrivate::openKey( const QString &key, REGSAM access, QString &value , bool global )
{
    HKEY res_key = NULL;
    HKEY scope;
    QString main_key;

    splitKey( key, main_key, value );
    searchPath( scope, main_key, access, global, 1 );

    QT_WA_INLINE(
        RegOpenKeyExW( scope, ( LPCTSTR ) main_key.ucs2(), 0, access, &res_key ),
        RegOpenKeyExA( scope, ( LPCSTR ) main_key.latin1(), 0, access, &res_key ) );

    return res_key;
}

void QSettingsSysPrivate::searchPath( HKEY &scope, QString &path, REGSAM access, bool global, bool readonly )
{
    const HKEY scopes[ 2 ] = {
                                 HKEY_CURRENT_USER,
                                 HKEY_LOCAL_MACHINE
                             };

    for ( QStringList::Iterator it = searchPaths.fromLast(); it != searchPaths.end(); --it ) {
        QString search_key;
        HKEY res_key;
        if ( ( *it ).length() )
            search_key = "Software\\" + ( *it );
        if ( path.length() )
            search_key += "\\" + path;
        search_key = search_key.replace( QRegExp( "(\\\\)+" ), "\\" );
        /* Now try to open this key */
        int i = global ? 1 : 0;
        for ( i ; i < 2; i++ ) {
            long ret;
            if ( readonly )
                ret = QT_WA_INLINE(
                          RegOpenKeyExW( scopes[ i ], ( LPCTSTR ) search_key.ucs2(),
                                         0, access, &res_key ),
                          RegOpenKeyExA( scopes[ i ], ( LPCSTR ) search_key.latin1(),
                                         0, access, &res_key ) );
            else
                ret = QT_WA_INLINE(
                          RegCreateKeyExW( scopes[ i ], ( LPCTSTR ) search_key.ucs2(), 0,
                                           NULL, REG_OPTION_NON_VOLATILE, access, NULL,
                                           &res_key , NULL ),
                          RegCreateKeyExA( scopes[ i ], ( LPCSTR ) search_key.latin1(), 0,
                                           NULL, REG_OPTION_NON_VOLATILE, access, NULL,
                                           &res_key , NULL ) );
            /* key found :) */
            if ( ret == ERROR_SUCCESS ) {
                RegCloseKey( res_key );
                scope = scopes[ i ];
                path = search_key;
                return ;
            }
        }
    }
    /* No key found -> use first path */
    scope = global ? scopes[ 1 ] : scopes[ 0 ];
    QString new_path = searchPaths.last();
    if ( new_path.length() && path.length() )
        new_path += "\\";
    new_path += path;
    while ( new_path[ 0 ] == '\\' )
        new_path.remove( 0, 1 );
    path = new_path;
}

void QSettingsSysPrivate::splitKey ( const QString &key, QString &main_key, QString &value )
{
    int pos = key.findRev( QRegExp( "(/|\\\\)" ) );
    if ( pos == -1 ) {
        main_key = QString( "" );
        value = key;
    } else {
        main_key = key.left( pos );
        value = key.right( key.length() - pos - 1 );
    }

    main_key.replace( '/', '\\' );
    while ( main_key[ 0 ] == '\\' )
        main_key.remove( 0, 1 );
    while ( main_key.right( 1 ) == "\\" )
        main_key.truncate( main_key.length() - 1 );
}


/* from qsettings_mac.cpp 3.3.2 */
void QSettingsPrivate::sysInit( void )
{
    sysd = new QSettingsSysPrivate;
    Q_CHECK_PTR( sysd );
    sysInsertSearchPath( QSettings::Windows, "" );
}

/* from qsettings_mac.cpp 3.3.2 */
void QSettingsPrivate::sysClear( void )
{
    delete sysd;
    sysd = NULL;
}

/* obsolete functions since already handled in public class
   QStringList sysReadListEntry( const QString &, bool * = 0 ) const;
   QStringList sysReadListEntry( const QString &, const QChar& sep, bool * = 0 ) const;
   bool sysWriteEntry( const QString &, const QStringList &, const QChar& sep )
   bool sysWriteEntry( const QString&key, const QStringList&data )
*/

#if !defined(Q_NO_BOOL_TYPE)
bool QSettingsPrivate::sysWriteEntry( const QString &key, bool data )
{
    return sysWriteEntry( key, ( int ) data );
}
#endif

bool QSettingsPrivate::sysWriteEntry( const QString &key, double data )
{
#ifdef QT_CHECK_STATE
    if ( key.isNull() || key.isEmpty() ) {
        qWarning( "QSettingsPrivate::sysWriteEntry: invalid null/empty key." );
        return false;
    }
#endif // QT_CHECK_STATE
    long ret = -1;
    QString value;

    HKEY hkey = sysd->createKey( key, KEY_SET_VALUE , value , globalScope );
    if ( hkey ) {
        ret = QT_WA_INLINE(
                  RegSetValueExW( hkey, ( LPCTSTR ) value.ucs2(), 0, REG_QWORD,
                                  ( LPBYTE ) & data, sizeof( double ) ),
                  RegSetValueExA( hkey, ( LPCSTR ) value.latin1(), 0, REG_QWORD,
                                  ( LPBYTE ) & data, sizeof( double ) ) );
        RegCloseKey( hkey );
    }
    return ( ret == ERROR_SUCCESS );
}

bool QSettingsPrivate::sysWriteEntry( const QString &key, int data )
{
#ifdef QT_CHECK_STATE
    if ( key.isNull() || key.isEmpty() ) {
        qWarning( "QSettingsPrivate::sysWriteEntry: invalid null/empty key." );
        return false;
    }
#endif // QT_CHECK_STATE
    long ret = -1;
    QString value;

    HKEY hkey = sysd->createKey( key, KEY_SET_VALUE , value , globalScope );
    if ( hkey ) {
        ret = QT_WA_INLINE(
                  RegSetValueExW( hkey, ( LPCTSTR ) value.ucs2(), 0, REG_DWORD,
                                  ( LPBYTE ) & data, sizeof( int ) ),
                  RegSetValueExA( hkey, ( LPCSTR ) value.latin1(), 0, REG_DWORD,
                                  ( LPBYTE ) & data, sizeof( int ) ) );
        RegCloseKey( hkey );
    }
    return ( ret == ERROR_SUCCESS );
}

bool QSettingsPrivate::sysWriteEntry( const QString &key, const QString &data )
{
#ifdef QT_CHECK_STATE
    if ( key.isNull() || key.isEmpty() ) {
        qWarning( "QSettingsPrivate::sysWriteEntry: invalid null/empty key." );
        return false;
    }
#endif // QT_CHECK_STATE
    long ret = -1;
    QString value;

    HKEY hkey = sysd->createKey( key, KEY_SET_VALUE , value , globalScope );
    if ( hkey ) {
        int len = ( data.length() + 1 ) * sizeof( TCHAR );
        if ( data.length() == 0 )        /* empty string! */
            len = 0;
        ret = QT_WA_INLINE(
                  RegSetValueExW( hkey, ( LPCTSTR ) value.ucs2(), 0, REG_SZ,
                                  ( LPBYTE ) data.ucs2(), len ),
                  RegSetValueExA( hkey, ( LPCSTR ) value.latin1(), 0, REG_SZ,
                                  ( LPBYTE ) data.latin1(), len ) );
        RegCloseKey( hkey );
    }
    return ( ret == ERROR_SUCCESS );
}

QStringList QSettingsPrivate::sysEntryList( const QString &key ) const
{
    QString value;
    QStringList list;

    HKEY hkey = sysd->openKey( key + "\\", KEY_QUERY_VALUE, value, globalScope );
    if ( hkey ) {
        int idx = 0;
        unsigned long count = QT_WA_INLINE( 16383, 260 );
        QByteArray ba( ( count + 1 ) * sizeof( TCHAR ) );

        while ( QT_WA_INLINE(
                    RegEnumValueW( hkey, idx, ( LPTSTR ) ba.data(),
                                   &count, NULL, NULL, NULL, NULL ),
                    RegEnumValueA( hkey, idx, ( LPSTR ) ba.data(),
                                   &count, NULL, NULL, NULL, NULL ) )
                == ERROR_SUCCESS ) {
            list.append ( QT_WA_INLINE( QString::fromUcs2( ( unsigned short * ) ba.data() ),
                                        QString::fromLatin1( ( LPCSTR ) ba.data() ) ) );
            idx++;
            count = QT_WA_INLINE( 16383, 260 );        /* !! */
        }
        RegCloseKey( hkey );
    }
    return list;
}

QStringList QSettingsPrivate::sysSubkeyList( const QString &key ) const
{
    QString value;
    QStringList list;

    HKEY hkey = sysd->openKey( key + "\\", KEY_ENUMERATE_SUB_KEYS, value, globalScope );
    if ( hkey ) {
        int idx = 0;
        unsigned long count = 255;
        QByteArray ba( ( count + 1 ) * sizeof( TCHAR ) );
        FILETIME time;

        while ( QT_WA_INLINE(
                    RegEnumKeyExW( hkey, idx, ( LPTSTR ) ba.data(),
                                   &count, NULL, NULL, NULL, &time ),
                    RegEnumKeyExA( hkey, idx, ( LPSTR ) ba.data(),
                                   &count, NULL, NULL, NULL, &time ) )
                == ERROR_SUCCESS ) {
            list.append ( QT_WA_INLINE( QString::fromUcs2( ( unsigned short * ) ba.data() ),
                                        QString::fromLatin1( ( LPCSTR ) ba.data() ) ) );
            idx++;
            count = 255;        /* !! */
        }
        RegCloseKey( hkey );
    }
    return list;
}

QString QSettingsPrivate::sysReadEntry( const QString &key, const QString &def, bool *ok ) const
{
#ifdef QT_CHECK_STATE
    if ( key.isNull() || key.isEmpty() ) {
        qWarning( "QSettingsPrivate::sysReadEntry: invalid null/empty key." );
        if ( ok )
            * ok = FALSE;
        return def;
    }
#endif // QT_CHECK_STATE
    QString value;

    if ( ok )
        * ok = FALSE;

    HKEY hkey = sysd->openKey( key, KEY_QUERY_VALUE , value, globalScope );
    if ( hkey ) {
        unsigned long ret, len = sizeof( int );

        /* First get the size of the data */
        ret = QT_WA_INLINE(
                  RegQueryValueExW( hkey, ( LPCTSTR ) value.ucs2(), NULL, NULL, NULL, &len ),
                  RegQueryValueExA( hkey, ( LPCSTR ) value.latin1(), NULL, NULL, NULL, &len ) );
        if ( ( ret != ERROR_SUCCESS ) || ( len == 0 ) ) {
            RegCloseKey( hkey );
            return def;
        }

        /* now create buffer to store data */
        QByteArray ba( len + 1 );   /* +1 to be sure :) */

        ret = QT_WA_INLINE(
                  RegQueryValueExW( hkey, ( LPCTSTR ) value.ucs2(), NULL, NULL,
                                    ( LPBYTE ) ba.data(), &len ),
                  RegQueryValueExA( hkey, ( LPCSTR ) value.latin1(), NULL, NULL,
                                    ( LPBYTE ) ba.data(), &len ) );
        RegCloseKey( hkey );
        if ( ( ret == ERROR_SUCCESS ) ) {
            QString str;

            str = QT_WA_INLINE( QString::fromUcs2( ( unsigned short * ) ba.data() ),
                                QString::fromLatin1( ( LPCSTR ) ba.data() ) );

            if ( ok )
                * ok = true;
            return str;
        }
    }
    return def;
}

int QSettingsPrivate::sysReadNumEntry( const QString &key, int def, bool * ok ) const
{
#ifdef QT_CHECK_STATE
    if ( key.isNull() || key.isEmpty() ) {
        qWarning( "QSettingsPrivate::sysReadNumEntry: invalid null/empty key." );
        if ( ok )
            * ok = false;
        return def;
    }
#endif // QT_CHECK_STATE
    QString value;

    if ( ok )
        * ok = false;

    HKEY hkey = sysd->openKey( key, KEY_QUERY_VALUE , value , globalScope );
    if ( hkey ) {
        int num;
        unsigned long ret, len = sizeof( int );

        ret = QT_WA_INLINE(
                  RegQueryValueExW( hkey, ( LPCTSTR ) value.ucs2(), NULL,
                                    NULL, ( LPBYTE ) & num, &len ),
                  RegQueryValueExA( hkey, ( LPCSTR ) value.latin1(), NULL,
                                    NULL, ( LPBYTE ) & num, &len ) );
        RegCloseKey( hkey );
        if ( ( ret == ERROR_SUCCESS ) && ( len == sizeof( int ) ) ) {
            if ( ok )
                * ok = true;
            return num;
        }
    }
    return def;
}

double QSettingsPrivate::sysReadDoubleEntry( const QString &key, double def, bool * ok ) const
{
#ifdef QT_CHECK_STATE
    if ( key.isNull() || key.isEmpty() ) {
        qWarning( "QSettingsPrivate::sysReadDoubleEntry: invalid null/empty key." );
        if ( ok )
            * ok = false;
        return def;
    }
#endif // QT_CHECK_STATE
    QString value;

    if ( ok )
        * ok = false;

    HKEY hkey = sysd->openKey( key, KEY_QUERY_VALUE , value, globalScope );
    if ( hkey ) {
        double num;
        unsigned long ret, len = sizeof( double );

        ret = QT_WA_INLINE(
                  RegQueryValueExW( hkey, ( LPCTSTR ) value.ucs2(), NULL,
                                    NULL, ( LPBYTE ) & num, &len ),
                  RegQueryValueExA( hkey, ( LPCSTR ) value.latin1(), NULL,
                                    NULL, ( LPBYTE ) & num, &len ) );
        RegCloseKey( hkey );
        if ( ( ret == ERROR_SUCCESS ) && ( len == sizeof( double ) ) ) {
            if ( ok )
                * ok = true;
            return num;
        }
    }
    return def;
}

bool QSettingsPrivate::sysReadBoolEntry( const QString &key, bool def, bool * ok ) const
{
    /* Since in sysWriteEntry(bool) bool gets converted to int, we must do the same here */
    return sysReadNumEntry( key, def, ok );
}

bool QSettingsPrivate::sysRemoveEntry( const QString &key )
{
    QString value;
    long ret = -1;

    HKEY hkey = sysd->openKey( key , KEY_SET_VALUE, value, globalScope );
    if ( hkey ) {
        /* This just deletes a value, not a subkey ... */
        ret = QT_WA_INLINE( RegDeleteValueW( hkey, ( LPCTSTR ) value.ucs2() ),
                            RegDeleteValueA( hkey, ( LPCSTR ) value.latin1() ) );
        RegCloseKey( hkey );
    }
    return ( ret == ERROR_SUCCESS );
}

/* what to do here? */
bool QSettingsPrivate::sysSync( void )
{
    return true;
}

/* from qsettings_mac.cpp 3.3.2 */
void QSettingsPrivate::sysInsertSearchPath( QSettings::System s, const QString &path )
{
    if ( s != QSettings::Windows )
        return ;
    if ( !path.isEmpty() && !qt_verify_key( path ) ) {
#if defined(QT_CHECK_STATE)
        qWarning( "QSettings::insertSearchPath: Invalid key: '%s'", path.isNull() ? "(null)" : path.latin1() );
#endif

        return ;
    }

    QString realpath = path;
    realpath.replace( '/', '\\' );
    while ( realpath.right( 1 ) == "\\" )
        realpath.truncate( realpath.length() - 1 );
    sysd->searchPaths.append( realpath );
}

/* from qsettings_mac.cpp 3.3.2 */
void QSettingsPrivate::sysRemoveSearchPath( QSettings::System s, const QString &path )
{
    if ( s != QSettings::Windows )
        return ;
    QString realpath = path;
    realpath.replace( '/', '\\' );
    while ( realpath.right( 1 ) == "\\" )
        realpath.truncate( realpath.length() - 1 );
    sysd->searchPaths.remove( realpath );
}

#endif //QT_NO_SETTINGS
