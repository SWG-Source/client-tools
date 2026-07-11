/****************************************************************************
** $Id: qmime_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of MIME support
**
** Created : 20030714
**
** Copyright (C) 2004 Ralf Habacker
** Copyright (C) 2004 Christian Ehrlicher
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#include "qmime.h"

#ifndef QT_NO_MIME

#include <shlobj.h>             // DROPFILES
#include "qapplication.h" // ### for now
#include "qbuffer.h"
#include "qclipboard.h" // ### for now
#include "qcleanuphandler.h"
#include "qdragobject.h"
#include "qmap.h"
#include "qregexp.h"
#include "qstring.h"
#include "qt_windows.h"

/* DIB <-> QImage convertors from qimage.cpp */
bool qt_read_dib( QDataStream& s, QImage& image );
bool qt_write_dib( QDataStream& s, QImage image );

QPtrList<QWindowsMime> mimes;
QMap<QString, int> mime_registry;

static void cleanup_mimes()        // post-routine
{
    QWindowsMime * wm;
    while ( ( wm = mimes.first() ) )
        delete wm;
}


QWindowsMime::QWindowsMime()
{
    mimes.append( this );
}

QWindowsMime::~QWindowsMime()
{
    mimes.remove( this );
}

QPtrList<QWindowsMime> QWindowsMime::all()
{
    QPtrList<QWindowsMime> ret;
    for ( QWindowsMime * wm = mimes.first(); wm; wm = mimes.next() ) {
        ret.append( wm );
    }
    return ret;
}

QWindowsMime* QWindowsMime::convertor( const char* mime, int cf )
{
    for ( QWindowsMime * wm = mimes.first(); wm; wm = mimes.next() ) {
        if ( wm->canConvert( mime, cf ) ) {
            return wm;
        }
    }
    return NULL;
}

const char* QWindowsMime::cfToMime( int cf )
{
    const char * m = NULL;
    for ( QWindowsMime * wm = mimes.first(); wm && !m; wm = mimes.next() ) {
        m = wm->mimeFor( cf );
        if ( m )
            return m;
    }
    return NULL;
}

int QWindowsMime::registerMimeType( const char *mime )
{
    int idx;

    if ( !mime_registry.contains( mime ) ) {
        QString mimetype ( mime );

        idx = QT_WA_INLINE ( RegisterClipboardFormatW( ( LPCWSTR ) mimetype.ucs2() ),
                             RegisterClipboardFormatA( ( LPCSTR ) mimetype.local8Bit() ) );
        if ( !idx )
            qWarning ( "Registering MIME-Type %s failed !", mime );

        mime_registry.insert( mime, idx );
    }
    return mime_registry[ mime ];
}
/*
    virtual const char* convertorName()=0;
    virtual int countCf()=0;
    virtual int cf(int index)=0;
    virtual bool canConvert( const char* mime, int cf )=0;
    virtual const char* mimeFor(int cf)=0;
    virtual int cfFor(const char* )=0;
    virtual QByteArray convertToMime( QByteArray data, const char* mime, int cf )=0;
    virtual QByteArray convertFromMime( QByteArray data, const char* mime, int cf )=0;
*/

class QWinMimeText : public QWindowsMime
{
public:
    QWinMimeText() : QWindowsMime()
    { }
    int countCf();
    const char* convertorName();
    int cf( int index );
    int cfFor( const char* mime );
    const char* mimeFor( int cf );
    bool canConvert( const char* mime, int cf );
    QByteArray convertToMime( QByteArray data, const char* , int );
    QByteArray convertFromMime( QByteArray data, const char* , int );
};

const char* QWinMimeText::convertorName()
{
    return "Text";
}

int QWinMimeText::countCf()
{
    return 2;
}

int QWinMimeText::cf( int index )
{
    if ( index == 0 )
        return CF_UNICODETEXT;
    return CF_TEXT;
}

bool QWinMimeText::canConvert( const char* mime, int cf )
{
    return ( cf && cfFor( mime ) == cf );
}

const char* QWinMimeText::mimeFor( int cf )
{
    if ( cf == CF_TEXT )
        return "text/plain";
    else if ( cf == CF_UNICODETEXT )
        return "text/plain;charset=ISO-10646-UCS-2";
    return NULL;
}

int QWinMimeText::cfFor( const char* mime )
{
    if ( !qstricmp( mime, "text/plain" ) )
        return CF_TEXT;
    QCString m( mime );
    int i = m.find( "charset=" );
    if ( i >= 0 ) {
        QCString cs( m.data() + i + 8 );
        i = cs.find( ";" );
        if ( i >= 0 )
            cs = cs.left( i );
        if ( cs == "system" )
            return CF_TEXT;
        if ( cs == "ISO-10646-UCS-2" || cs == "utf16" )
            return CF_UNICODETEXT;
    }
    return 0;
}

QByteArray QWinMimeText::convertToMime( QByteArray data, const char* mime, int cf )
{
    /* I wonder why we can't simply "return data" like in qmime_mac */
    /* Valid cf? */
    if ( ( cf != CF_TEXT ) && ( cf != CF_UNICODETEXT ) )
        return QByteArray();
    /* only CF_TEXT -> plain/text / system and CF_UNICODETEXT -> utf16 / ISO-10646-UCS-2 */
    if ( cf != cfFor( mime ) )
        return QByteArray();

    QByteArray ret;
    /* Plain Text -> http://www.rfc-editor.org/rfc/rfc2046.txt
       Representation of Line Breaks:
       The canonical form of any MIME "text" subtype MUST
       always represent a line break as a CRLF sequence.

       So our only problem is when we want to convert text with
       lf only to mime
       Qt 3.2.1 non commercial convert crlf to lf! wtf? */
    if ( cf == CF_TEXT ) {
        int datasize = data.size();
        ret.resize( datasize, QGArray::SpeedOptim );
        int offset = 0;         /* for possible extra cr's */
        int already_0d = false;
        for ( int i = 0; i < (int)data.size(); i++ ) {
            int c = data[ i ];
            /* lf -> crlf */
            if ( c == 0x0d ) {
                already_0d = true;
            } else if ( c == 0x0a ) {
                if ( !already_0d ) {
                    ret[ i + offset ] = 0x0d;
                    offset++;
                    datasize++;
                    ret.resize( datasize, QGArray::SpeedOptim );
                    already_0d = false;
                }
            } else if ( already_0d ) {
                already_0d = false;
            }
            ret[ i + offset ] = c;
        }
        int size = ret.size();
        char temp = *((char*)&ret[size - 1]);
        while ( ( temp == 0 ) || ( temp == 0x0a ) || ( temp == 0x0d ) ) {
            size -=1;
            ret.resize( size , QGArray::SpeedOptim );
            temp = *((char*)&ret[size - 1]);
        }
    } else if ( cf == CF_UNICODETEXT ) {
        /* We convert from little endian - do we?
           I think Qt is using little endian under win32, and MS too:
           http://msdn.microsoft.com/library/default.asp?url=/library/en-us/intl/unicode_42jv.asp */
        int mySize = 0;
        for( ; mySize + 1 < (int)data.size(); mySize += 2) {
            if( data[mySize] == '\0' && data[mySize + 1] == '\0' )
                break;
        }
        ret.resize( mySize + 2, QGArray::SpeedOptim );
        ret[ 0 ] = (BYTE)0xFF;
        ret[ 1 ] = (BYTE)0xFE;
        memcpy( ret.data() + 2, data.data(), mySize );
    }
    return ret;
}

QByteArray QWinMimeText::convertFromMime( QByteArray data, const char* mime, int cf )
{
    /* Valid cf? */
    if ( ( cf != CF_TEXT ) && ( cf != CF_UNICODETEXT ) )
        return QByteArray();
    /* only CF_TEXT -> plain/text / system and CF_UNICODETEXT -> utf16 / ISO-10646-UCS-2 */
    if ( cf != cfFor( mime ) )
        return QByteArray();

    QByteArray ret( data.size() );

    int size; /* stupid msvc ... */
    if ( cf == CF_TEXT ) {
        /* Don't know if we should convert crlf to lf, but this shouldn't be that problem
           Qt 3.2.1 non commercial converts lf to crlf */
        /* Although plain/text shouldn't contain single lf's we search for them... */
        int offset = 0;         /* for possible extra cr's */
        int already_0d = false;
        size = data.size();
        for ( int i = 0; i < (int)data.size(); i++ ) {
            int c = data[ i ];
            /* lf -> crlf */
            if ( c == 0x0d ) {
                already_0d = true;
            } else if ( c == 0x0a ) {
                if ( !already_0d ) {
                    ret[ i + offset ] = 0x0d;
                    offset++;
                    size++;
                    ret.resize( size, QGArray::SpeedOptim );
                    already_0d = false;
                }
            } else if ( already_0d ) {
                already_0d = false;
            }
            ret[ i + offset ] = c;
        }
        size = ret.size();
        if ( ret[ size - 1 ] != '\0' ) {
            ret.resize( size + 1 , QGArray::SpeedOptim );
            ret[ size ] = '\0';
        }

    } else if ( cf == CF_UNICODETEXT ) {
        /* The first short defines little or big endian */
        /* This idea is from Mono -> UnicodeEncoding.cs */
        int offset = 0;
        bool isBigEndian = false;
        if ( data.size() > 2 ) {
            unsigned char b1 = data [ 0 ];
            unsigned char b2 = data [ 1 ];
            if ( ( b1 == 0xfe ) && ( b2 == 0xff ) ) {
                isBigEndian = true;
                offset = 2;
            } else if ( ( b1 == 0xff ) && ( b2 == 0xfe ) ) {
                isBigEndian = false;
                offset = 2;
            }
            /* We need 2 Bytes less */
            ret.resize( data.size() - offset, QGArray::SpeedOptim );
        }
        int len = data.size() - offset;
        if ( isBigEndian ) {
            /* What if data.size() % 2 == 1 ? */

            for ( int i = 0; i < len - 1; i += 2 ) {
                ret[ i ] = data[ i + offset + 1 ];
                ret[ i + 1 ] = data[ i + offset ];
            }
        } else {
            for ( int i = 0; i < len - 1; i += 2 ) {
                ret[ i ] = data[ i + offset ];
                ret[ i + 1 ] = data[ i + offset + 1 ];
            }
        }
        size = ret.size();
        if ( !( ( ret[ size - 1 ] == '\0' ) && ( ret[ size - 2 ] == '\0' ) ) ) {
            ret.resize( size + 2 , QGArray::SpeedOptim );
            ret[ size ] = '\0';
            ret[ size + 1 ] = '\0';
        }
    }
    return ret;
}

class QWinMimeDIB : public QWindowsMime
{
public:
    QWinMimeDIB() : QWindowsMime()
    { }
    int countCf();
    const char* convertorName();
    int cf( int index );
    int cfFor( const char* mime );
    const char* mimeFor( int cf );
    bool canConvert( const char* mime, int cf );
    QByteArray convertToMime( QByteArray data, const char* , int );
    QByteArray convertFromMime( QByteArray data, const char* , int );
};

int QWinMimeDIB::countCf()
{
    return 1;
}

const char* QWinMimeDIB::convertorName()
{
    return "Image";
}

int QWinMimeDIB::cf( int )
{
    return CF_DIB;
}

int QWinMimeDIB::cfFor( const char* mime )
{
    if ( !qstrnicmp( mime, "image/", 5 ) ) {
        QStrList ofmts = QImage::outputFormats();
        for ( const char * fmt = ofmts.first(); fmt; fmt = ofmts.next() ) {
            if ( !qstricmp( fmt, mime + 6 ) )
                return CF_DIB;
        }
    }
    return 0;
}

const char* QWinMimeDIB::mimeFor( int cf )
{
    if ( cf == CF_DIB )
        return "image/png";
    return 0;
}

bool QWinMimeDIB::canConvert( const char* mime, int cf )
{
    if ( cf == CF_DIB && !qstrnicmp( mime, "image/", 5 ) ) {
        QStrList ofmts = QImage::outputFormats();
        for ( const char * fmt = ofmts.first(); fmt; fmt = ofmts.next() ) {
            if ( !qstricmp( fmt, mime + 6 ) )
                return true;
        }
    }
    return false;
}

QByteArray QWinMimeDIB::convertToMime( QByteArray data, const char* mime, int cf )
{
    QByteArray ret;
    if ( qstrnicmp( mime, "image/", 6 ) || cf != CF_DIB )
        return ret;

    QImage img;
    QString format = QString( mime ).remove( "image/" );

    QBuffer buf( data );
    buf.open( IO_ReadOnly );
    QDataStream ds ( &buf );

    // " The default setting is big endian" ...
    ds.setByteOrder( QDataStream::LittleEndian );
    if ( !qt_read_dib ( ds, img ) )
        return ret;
    buf.close();

    buf.setBuffer( ret );
    buf.open( IO_WriteOnly );
    QImageIO iio( &buf, format.upper().latin1() );

    iio.setImage( img );
    iio.write();

    buf.close();

    return ret;
}

QByteArray QWinMimeDIB::convertFromMime( QByteArray data, const char* mime, int cf )
{
    QByteArray ret;
    if ( qstrnicmp( mime, "image/", 6 ) || cf != CF_DIB )
        return ret;

    QImage img;
    QString format = QString( mime ).remove( "image/" );

    img.loadFromData( ( unsigned char* ) data.data(), data.size(), format.upper().latin1() );

    QBuffer buf( ret );
    buf.open( IO_WriteOnly );
    QDataStream ds ( &buf );

    // " The default setting is big endian" ...
    ds.setByteOrder( QDataStream::LittleEndian );
    if ( !qt_write_dib ( ds, img ) )
        return ret;
    buf.close();

    return ret;
}

class QWinMimeHdrop : public QWindowsMime
{
public:
    QWinMimeHdrop() : QWindowsMime()
    { }
    int countCf();
    const char* convertorName();
    int cf( int index );
    int cfFor( const char* mime );
    const char* mimeFor( int cf );
    bool canConvert( const char* mime, int cf );
    QByteArray convertToMime( QByteArray data, const char* , int );
    QByteArray convertFromMime( QByteArray data, const char* , int );
};

int QWinMimeHdrop::countCf()
{
    return 1;
}

const char* QWinMimeHdrop::convertorName()
{
    return "HDrop";
}

int QWinMimeHdrop::cf( int )
{
    return CF_HDROP;
}

int QWinMimeHdrop::cfFor( const char* mime )
{
    if ( qstricmp( mime, "text/uri-list" ) )
        return 0;
    return CF_HDROP;
}

const char* QWinMimeHdrop::mimeFor( int cf )
{
    if ( cf == CF_HDROP )
        return "text/uri-list";
    return NULL;
}

bool QWinMimeHdrop::canConvert( const char* mime, int cf )
{
    if ( !qstricmp( mime, "text/uri-list" ) )
        return cf == CF_HDROP;
    return FALSE;
}

QByteArray QWinMimeHdrop::convertToMime( QByteArray data, const char* mime, int cf )
{
    QByteArray ret;

    if ( qstricmp( mime, "text/uri-list" ) || cf != CF_HDROP )
        return ret;

    /* We get a DROPFILES structure with apended files and *not* a HDROP ! */

    DROPFILES *df = ( DROPFILES * ) data.data();

    QStringList strlist;
    QBuffer buf ( ret );
    buf.open ( IO_WriteOnly );
    const char *ptr = ( ( const char* ) df ) + df->pFiles;
    const char *end = ( ( const char* ) data.data() ) + data.size();
    if ( df->fWide ) {  /* Unicode */
        WCHAR * wptr = ( WCHAR* ) ptr;
        WCHAR * wend = ( WCHAR* ) end;
        while ( ( *wptr != '\0' ) && ( wptr <= wend ) ) {
            QString filename = qt_winQString( wptr );
            strlist.append( QString( "file:///" ) + filename );
            wptr += filename.length() + 1;
        }
    } else {    /* Ansi */
        while ( ( *ptr != '\0' ) && ( ptr <= end ) ) {
            QString filename = QString( ptr );
            strlist.append( QString( "file:///" ) + filename );
            ptr += filename.length() + 1;
        }
    }
    buf.writeBlock( strlist.join( "\r\n" ).local8Bit() );
    buf.writeBlock( "\0", 1 );
    buf.close();

    return ret;
}

QByteArray QWinMimeHdrop::convertFromMime( QByteArray data, const char* mime, int cf )
{
    QByteArray ret;
    if ( qstricmp( mime, "text/uri-list" ) || cf != CF_HDROP )
        return ret;
    /* We must fill a DROPFILES structure and return a handle to this structure
       -> HDROP */
    /* data is 8 bit encoded */
    int start = 0;
    QString str = QString::fromAscii( data.data(), data.size() );
    QStringList strlist = QStringList::split( QRegExp( "\n|\r\n" ), str );
    DROPFILES df;
    df.pFiles = sizeof( DROPFILES );
    df.fNC = false;
    df.fWide = QT_WA_INLINE ( true, false );

    QBuffer buf ( ret );
    buf.open ( IO_ReadWrite );
    buf.writeBlock( ( char* ) & df, sizeof( DROPFILES ) );
    for ( int i = 0; i < (int)strlist.count(); i++ ) {
        strlist[ i ] = strlist[ i ].remove( "file:///" );
        QT_WA(
            buf.writeBlock( ( char* ) strlist[ i ].ucs2(), ( strlist[ i ].length() + 1 ) * 2 );
            ,
            buf.writeBlock( strlist[ i ].latin1(), strlist[ i ].length() + 1 );
        )
    }
    buf.writeBlock( "\0\0\0\0", 4 );  /* better some more :-D */
    buf.close();

    return ret;
}

class QWinMimeAnyMime : public QWindowsMime
{
public:
    QWinMimeAnyMime() : QWindowsMime()
    { }
    int countCf();
    const char* convertorName();
    int cf( int index );
    int cfFor( const char* mime );
    const char* mimeFor( int cf );
    bool canConvert( const char* mime, int cf );
    QByteArray convertToMime( QByteArray data, const char* , int );
    QByteArray convertFromMime( QByteArray data, const char* , int );
};

int QWinMimeAnyMime::countCf()
{
    return mime_registry.count();
}

const char* QWinMimeAnyMime::convertorName()
{
    return "Any-Mime";
}

int QWinMimeAnyMime::cf( int index )
{
    int i = 0;
    for ( QMap<QString, int>::Iterator it = mime_registry.begin(); it != mime_registry.end(); ++it, ++i ) {
        if ( i == index )
            return it.data();
    }
    return 0;
}

int QWinMimeAnyMime::cfFor( const char* mime )
{
    return registerMimeType( mime );
}

const char* QWinMimeAnyMime::mimeFor( int cf )
{
    for ( QMap<QString, int>::Iterator it = mime_registry.begin(); it != mime_registry.end(); ++it ) {
        if ( it.data() == cf )
            return it.key();
    }
    return NULL;
}

bool QWinMimeAnyMime::canConvert( const char* mime, int cf )
{
    if ( mime_registry.contains( mime ) && mime_registry[ mime ] == cf )
        return TRUE;
    return FALSE;
}

QByteArray QWinMimeAnyMime::convertToMime( QByteArray data, const char* , int )
{
    return data;
}

QByteArray QWinMimeAnyMime::convertFromMime( QByteArray data, const char* , int )
{
    return data;
}


/* Otherwise it won't work */
void QWindowsMime::initialize()
{
    if ( mimes.isEmpty() ) {
        qAddPostRoutine( cleanup_mimes );
        new QWinMimeDIB;
        new QWinMimeText;
        new QWinMimeHdrop;
        new QWinMimeAnyMime;
    }
}

#endif // QT_NO_MIME
