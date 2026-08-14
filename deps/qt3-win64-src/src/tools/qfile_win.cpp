/****************************************************************************
** $Id: qfile_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QFileInfo class
**
** Created : 20030709
**
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2004 Ralf Habacker
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

#include "qplatformdefs.h"
#include "qfile.h"

#include <qt_windows.h>

extern const char* qt_fileerr_read;

/* replace '/' with '\\' and use local encoding */
static inline QString qt_reslashify( const QString& fn )
{
    QString filename = fn;
    return ( filename.replace( '/', '\\' ) );
}

bool qt_file_access( const QString& fileName, int t )
{
    if ( fileName.isEmpty() ) {
#if defined(QT_CHECK_NULL)
        qWarning( "QFile::remove: Empty or null file name" );
#endif
        return FALSE;
    }

#if defined(Q_CC_MSVC)
    if ( !( t==0 || t==2 || t==4 || t==6 ) )
        t = 0;
#endif
    QString str = qt_reslashify( fileName );
    QT_WA( 
        return ( _waccess( (wchar_t*) str.ucs2(), t ) == 0 );
        ,
        return ( _access( QFile::encodeName( str ), t ) == 0 );
    )
}

/*!
    \overload
    Removes the file \a fileName.
  Returns TRUE if successful, otherwise FALSE.
*/

bool QFile::remove ( const QString &fileName )
{
    if ( fileName.isEmpty() ) {
#if defined(QT_CHECK_NULL)
        qWarning( "QFile::remove: Empty or null file name" );
#endif
        return FALSE;
    }
    QString str = qt_reslashify( fileName );
    QT_WA(
        return ( _wunlink( (wchar_t*) str.ucs2() ) == 0 );
        ,
        return ( _unlink( QFile::encodeName( str ) ) == 0 );
    )
}

#if defined(O_NONBLOCK)
# define HAS_ASYNC_FILEMODE
# define OPEN_ASYNC O_NONBLOCK
#elif defined(O_NDELAY)
# define HAS_ASYNC_FILEMODE
# define OPEN_ASYNC O_NDELAY
#endif

/*!
  Opens the file specified by the file name currently set, using the mode \a m.
  Returns TRUE if successful, otherwise FALSE.

  \keyword IO_Raw
  \keyword IO_ReadOnly
  \keyword IO_WriteOnly
  \keyword IO_ReadWrite
  \keyword IO_Append
  \keyword IO_Truncate
  \keyword IO_Translate

  The mode parameter \a m must be a combination of the following flags:
  \list
  \i \c IO_Raw specified raw (non-buffered) file access.
  \i \c IO_ReadOnly opens the file in read-only mode.
  \i \c IO_WriteOnly opens the file in write-only mode (and truncates).
  \i \c IO_ReadWrite opens the file in read/write mode, equivalent to
     \c (IO_ReadOnly | IO_WriteOnly).
  \i \c IO_Append opens the file in append mode. This mode is very useful
  when you want to write something to a log file. The file index is set to
  the end of the file. Note that the result is undefined if you position the
  file index manually using at() in append mode.
  \i \c IO_Truncate truncates the file.
  \i \c IO_Translate enables carriage returns and linefeed translation
  for text files under MS-DOS, Windows and OS/2.
  \endlist

  The raw access mode is best when I/O is block-operated using 4kB block size
  or greater. Buffered access works better when reading small portions of
  data at a time.

  <strong>Important:</strong> When working with buffered files, data may
  not be written to the file at once. Call flush()
  to make sure that the data is really written.


    \warning If you have a buffered file opened for both reading and
    writing you must not perform an input operation immediately after
    an output operation or vice versa. You should always call flush()
    or a file positioning operation, e.g. seek(), between input and
    output operations, otherwise the buffer may contain garbage.

  If the file does not exist and \c IO_WriteOnly or \c IO_ReadWrite is
  specified, it is created.

  Example:
  \code
    QFile f1( "/tmp/data.bin" );
    QFile f2( "readme.txt" );
    f1.open( IO_Raw | IO_ReadWrite | IO_Append );
    f2.open( IO_ReadOnly | IO_Translate );
  \endcode

  \sa name(), close(), isOpen(), flush()
*/

#define HAS_TEXT_FILEMODE

bool QFile::open( int m )
{
    if ( isOpen() ) {    // file already open
#if defined(QT_CHECK_STATE)
        qWarning( "QFile::open: File already open" );
#endif

        return FALSE;
    }
    if ( fn.isEmpty() ) {   // no file name defined
#if defined(QT_CHECK_NULL)
        qWarning( "QFile::open: No file name specified" );
#endif

        return FALSE;
    }
    init();     // reset params
    setMode( m );
    if ( !( isReadable() || isWritable() ) ) {
#if defined(QT_CHECK_RANGE)
        qWarning( "QFile::open: File access not specified" );
#endif

        return FALSE;
    }
    bool ok = TRUE;
    QT_STATBUF st;
    if ( isRaw() ) {    // raw file I/O
        int oflags = QT_OPEN_RDONLY;
        if ( isReadable() && isWritable() )
            oflags = QT_OPEN_RDWR;
        else if ( isWritable() )
            oflags = QT_OPEN_WRONLY;
        if ( flags() & IO_Append ) {  // append to end of file?
            if ( flags() & IO_Truncate )
                oflags |= ( QT_OPEN_CREAT | QT_OPEN_TRUNC );
            else
                oflags |= ( QT_OPEN_APPEND | QT_OPEN_CREAT );
            setFlags( flags() | IO_WriteOnly ); // append implies write
        } else if ( isWritable() ) {  // create/trunc if writable
            if ( flags() & IO_Truncate )
                oflags |= ( QT_OPEN_CREAT | QT_OPEN_TRUNC );
            else
                oflags |= QT_OPEN_CREAT;
        }
#if defined(HAS_TEXT_FILEMODE)
        if ( isTranslated() )
            oflags |= QT_OPEN_TEXT;
        else
            oflags |= QT_OPEN_BINARY;
#endif
#if defined(HAS_ASYNC_FILEMODE)

        if ( isAsynchronous() )
            oflags |= OPEN_ASYNC;
#endif

        QString str = qt_reslashify( fn );
#ifdef Q_CC_BOR
        QT_WA(
            fd = _wopen( (wchar_t*) str.ucs2(), oflags, 0666 );
            ,
            fd = _open( QFile::encodeName( str ), oflags );
        )
#else
        QT_WA(
            fd = _wopen( (wchar_t*) str.ucs2(), oflags, 0666 );
            ,
            fd = _open( QFile::encodeName( str ), oflags, 0666 );
        )
#endif

        if ( fd != -1 ) {   // open successful
            QT_FSTAT( fd, &st ); // get the stat for later usage
        } else {
            ok = FALSE;
        }
    } else {     // buffered file I/O
        QCString perm;
        char perm2[ 4 ];
        bool try_create = FALSE;
        if ( flags() & IO_Append ) {  // append to end of file?
            setFlags( flags() | IO_WriteOnly ); // append implies write
            perm = isReadable() ? "a+" : "a";
        } else {
            if ( isReadWrite() ) {
                if ( flags() & IO_Truncate ) {
                    perm = "w+";
                } else {
                    perm = "r+";
                    try_create = TRUE;  // try to create if not exists
                }
            } else if ( isReadable() ) {
                perm = "r";
            } else if ( isWritable() ) {
                perm = "w";
            }
        }
        qstrcpy( perm2, perm );
#if defined(HAS_TEXT_FILEMODE)

        if ( isTranslated() )
            strcat( perm2, "t" );
        else
            strcat( perm2, "b" );
#endif

        for ( ;; ) { // At most twice

            QString str = qt_reslashify( fn );
            QString prm( perm2 );
            QT_WA(
                fh = _wfopen( (wchar_t*) str.ucs2(), (wchar_t*) prm.ucs2() );
                ,
                fh = fopen( QFile::encodeName( str ), prm.latin1() );
            )

            if ( !fh && try_create ) {
                perm2[ 0 ] = 'w';   // try "w+" instead of "r+"
                try_create = FALSE;
            } else {
                break;
            }
        }
        if ( fh ) {
            QT_FSTAT( QT_FILENO( fh ), &st ); // get the stat for later usage
        } else {
            ok = FALSE;
        }
    }
    if ( ok ) {
        setState( IO_Open );
        // on successful open the file stat was got; now test what type
        // of file we have
        if ( ( st.st_mode & QT_STAT_MASK ) != QT_STAT_REG ) {
            // non-seekable
            setType( IO_Sequential );
            length = INT_MAX;
            ioIndex = 0;
        } else {
            length = ( int ) st.st_size;
            ioIndex = ( flags() & IO_Append ) == 0 ? 0 : length;
            if ( !( flags() & IO_Truncate ) && length == 0 && isReadable() ) {
                // try if you can read from it (if you can, it's a sequential
                // device; e.g. a file in the /proc filesystem)
                int c = getch();
                if ( c != -1 ) {
                    ungetch( c );
                    setType( IO_Sequential );
                    length = INT_MAX;
                    ioIndex = 0;
                }
            }
        }
    } else {
        init();
        if ( errno == EMFILE )    // no more file handles/descrs
            setStatus( IO_ResourceError );
        else
            setStatus( IO_OpenError );
    }
    return ok;
}

/*!
    \overload
  Opens a file in the mode \a m using an existing file handle \a f.
  Returns TRUE if successful, otherwise FALSE.

  Example:
  \code
    #include <stdio.h>

    void printError( const char* msg )
    {
        QFile f;
        f.open( IO_WriteOnly, stderr );
        f.writeBlock( msg, qstrlen(msg) ); // write to stderr
        f.close();
    }
  \endcode

  When a QFile is opened using this function, close() does not actually
  close the file, only flushes it.

  \warning If \a f is \c stdin, \c stdout, \c stderr, you may not
  be able to seek.  See QIODevice::isSequentialAccess() for more
  information.

  \sa close()
*/

bool QFile::open( int m, FILE *f )
{
    if ( isOpen() ) {
#if defined(QT_CHECK_RANGE)
        qWarning( "QFile::open: File already open" );
#endif

        return FALSE;
    }
    init();
    setMode( m & ~IO_Raw );
    setState( IO_Open );
    fh = f;
    ext_f = TRUE;
    QT_STATBUF st;
    QT_FSTAT( QT_FILENO( fh ), &st );
    ioIndex = ( int ) ftell( fh );
    if ( ( st.st_mode & QT_STAT_MASK ) != QT_STAT_REG || f == stdin ) { //stdin is non seekable
        // non-seekable
        setType( IO_Sequential );
        length = INT_MAX;
        ioIndex = 0;
    } else {
        length = ( Offset ) st.st_size;
        if ( !( flags() & IO_Truncate ) && length == 0 && isReadable() ) {
            // try if you can read from it (if you can, it's a sequential
            // device; e.g. a file in the /proc filesystem)
            int c = getch();
            if ( c != -1 ) {
                ungetch( c );
                setType( IO_Sequential );
                length = INT_MAX;
                ioIndex = 0;
            }
            resetStatus();
        }
    }
    return TRUE;
}

/*!
    \overload
  Opens a file in the mode \a m using an existing file descriptor \a f.
  Returns TRUE if successful, otherwise FALSE.

  When a QFile is opened using this function, close() does not actually
  close the file.

  The QFile that is opened using this function, is automatically set to be in
  raw mode; this means that the file input/output functions are slow. If you
  run into performance issues, you should try to use one of the other open
  functions.

  \warning If \a f is one of 0 (stdin), 1 (stdout) or 2 (stderr), you may not
  be able to seek. size() is set to \c INT_MAX (in limits.h).

  \sa close()
*/


bool QFile::open( int m, int f )
{
    if ( isOpen() ) {
#if defined(QT_CHECK_RANGE)
        qWarning( "QFile::open: File already open" );
#endif

        return FALSE;
    }
    init();
    setMode( m | IO_Raw );
    setState( IO_Open );
    fd = f;
    ext_f = TRUE;
    QT_STATBUF st;
    QT_FSTAT( fd, &st );
    ioIndex = ( int ) QT_LSEEK( fd, 0, SEEK_CUR );
    if ( ( st.st_mode & QT_STAT_MASK ) != QT_STAT_REG || f == 0 ) { // stdin is not seekable...
        // non-seekable
        setType( IO_Sequential );
        length = INT_MAX;
        ioIndex = 0;
    } else {
        length = ( Offset ) st.st_size;
        if ( length == 0 && isReadable() ) {
            // try if you can read from it (if you can, it's a sequential
            // device; e.g. a file in the /proc filesystem)
            int c = getch();
            if ( c != -1 ) {
                ungetch( c );
                setType( IO_Sequential );
                length = INT_MAX;
                ioIndex = 0;
            }
            resetStatus();
        }
    }
    return TRUE;
}

/*!
  Returns the file size.
  \sa at()
*/

QIODevice::Offset QFile::size() const
{
    if ( isOpen() ) {
        QT_STATBUF st;
        if ( QT_FSTAT( fh ? QT_FILENO( fh ) : fd, &st ) != -1 )
            return st.st_size;
    } else {
        QT_STATBUF st;
        QString str = qt_reslashify( fn );
#ifdef QT_LARGEFILE_SUPPORT
        if ( QT_WA_INLINE( _wstati64( (wchar_t*) str.ucs2(), &st ),
                           _stati64( QFile::encodeName( str ), &st ) ) != -1 ) {
#else
        if ( QT_WA_INLINE( _wstat( (wchar_t*) str.ucs2(), &st ),
                           _stat( QFile::encodeName( str ), &st ) ) != -1 ) {
#endif
            return st.st_size;
        }
    }
    return 0;
}


/*!
  Sets the file index to \a pos. Returns TRUE if successful; otherwise
  returns FALSE.

  Example:
  \code
    QFile f( "data.bin" );
    f.open( IO_ReadOnly );			// index set to 0
    f.at( 100 );                    // set index to 100
    f.at( f.at()+50 );    			// set index to 150
    f.at( f.size()-80 );   			// set index to 80 before EOF
    f.close();
  \endcode

  Use \c at() without arguments to retrieve the file offset.

  \warning The result is undefined if the file was open()'ed
  using the \c IO_Append specifier.

  \sa size(), open()
*/

bool QFile::at( Offset pos )
{
    if ( !isOpen() ) {
#if defined(QT_CHECK_STATE)
        qWarning( "QFile::at: File is not open" );
#endif

        return FALSE;
    }
    if ( isSequentialAccess() )
        return FALSE;
    bool ok;
    if ( isRaw() ) {    // raw file
        off_t l = ::lseek( fd, pos, SEEK_SET );
        ok = ( l != -1 );
        pos = ( Offset ) l;
    } else {     // buffered file
        ok = ( ::fseek( fh, pos, SEEK_SET ) == 0 );
    }
    if ( ok )
        ioIndex = ( Offset ) pos;
#if defined(QT_CHECK_RANGE)

    else
        qWarning( "QFile::at: Cannot set file position %lu", pos );
#endif

    return ok;
}

/*!
  Reads at most \a len bytes from the file into \a p and returns the
  number of bytes actually read.

  Returns -1 if a serious error occurred.

  \warning We have experienced problems with some C libraries when a buffered
  file is opened for both reading and writing. If a read operation takes place
  immediately after a write operation, the read buffer contains garbage data.
  Worse, the same garbage is written to the file. Calling flush() before
  readBlock() solved this problem.

  \sa writeBlock()
*/

Q_LONG QFile::readBlock( char *p, Q_ULONG len )
{
    if ( !len )  // nothing to do
        return 0;

#if defined(QT_CHECK_NULL)

    if ( !p )
        qWarning( "QFile::readBlock: Null pointer error" );
#endif
#if defined(QT_CHECK_STATE)

    if ( !isOpen() ) {    // file not open
        qWarning( "QFile::readBlock: File not open" );
        return -1;
    }
    if ( !isReadable() ) {   // reading not permitted
        qWarning( "QFile::readBlock: Read operation not permitted" );
        return -1;
    }
#endif
    Q_ULONG nread = 0;     // number of bytes read
    if ( !ungetchBuffer.isEmpty() ) {
        // need to add these to the returned string.
        uint l = ungetchBuffer.length();
        while ( nread < l ) {
            *p = ungetchBuffer.at( l - nread - 1 );
            p++;
            nread++;
        }
        ungetchBuffer.truncate( l - nread );
    }

    if ( nread < len ) {
        if ( isRaw() ) {    // raw file
            nread += QT_READ( fd, p, len - nread );
            if ( len && nread <= 0 ) {
                nread = 0;
                setStatus( IO_ReadError );
                setErrorStringErrno( errno );
            }
        } else {     // buffered file
            nread += fread( p, 1, len - nread, fh );
            if ( ( uint ) nread != len ) {
                if ( ferror( fh ) || nread == 0 )
                    setStatus( IO_ReadError );
                setErrorString( qt_fileerr_read );
            }
        }
    }
    if ( !isSequentialAccess() )
        ioIndex += nread;
    return nread;
}

/*! \overload Q_ULONG QFile::writeBlock( const QByteArray& data )
*/

/*! \reimp

  Writes \a len bytes from \a p to the file and returns the number of
  bytes actually written.

  Returns -1 if a serious error occurred.

  \warning When working with buffered files, data may not be written
  to the file at once. Call flush() to make sure the data is really
  written.

  \sa readBlock()
*/

Q_LONG QFile::writeBlock( const char *p, Q_ULONG len )
{
    if ( !len )  // nothing to do
        return 0;

#if defined(QT_CHECK_NULL)

    if ( p == 0 && len != 0 )
        qWarning( "QFile::writeBlock: Null pointer error" );
#endif
#if defined(QT_CHECK_STATE)

    if ( !isOpen() ) {    // file not open
        qWarning( "QFile::writeBlock: File not open" );
        return -1;
    }
    if ( !isWritable() ) {   // writing not permitted
        qWarning( "QFile::writeBlock: Write operation not permitted" );
        return -1;
    }
#endif
    Q_ULONG nwritten;    // number of bytes written
    if ( isRaw() )       // raw file
        nwritten = QT_WRITE( fd, ( void * ) p, len );
    else     // buffered file
        nwritten = fwrite( p, 1, len, fh );
    if ( nwritten != len ) {  // write error
        if ( errno == ENOSPC )     // disk is full
            setStatus( IO_ResourceError );
        else
            setStatus( IO_WriteError );
        setErrorStringErrno( errno );
        if ( !isSequentialAccess() ) {
            if ( isRaw() )      // recalc file position
                ioIndex = ( Offset ) QT_LSEEK( fd, 0, SEEK_CUR );
            else
                ioIndex = ( Offset ) ::fseek( fh, 0, SEEK_CUR );
        }
    } else {
        if ( !isSequentialAccess() )
            ioIndex += nwritten;
    }
    if ( ioIndex > length )     // update file length
        length = ioIndex;
    return nwritten;
}

/*!
  Returns the file handle of the file.

  This is a small positive integer, suitable for use with C library
  functions such as fdopen() and fcntl(), as well as with QSocketNotifier.

  If the file is not open or there is an error, handle() returns -1.

  \sa QSocketNotifier
*/

int QFile::handle() const
{
    if ( !isOpen() )
        return -1;
    else if ( fh )
        return QT_FILENO( fh );
    else
        return fd;
}

/*!
  Closes an open file.

  The file is not closed if it was opened with an existing file handle.
  If the existing file handle is a \c FILE*, the file is flushed.
  If the existing file handle is an \c int file descriptor, nothing
  is done to the file.

  Some "write-behind" filesystems may report an unspecified error on
  closing the file. These errors only indicate that something may
  have gone wrong since the previous open(). In such a case status()
  reports IO_UnspecifiedError after close(), otherwise IO_Ok.

  \sa open(), flush()
*/


void QFile::close()
{
    bool ok = FALSE;
    if ( isOpen() ) {    // file is not open
        if ( fh ) {    // buffered file
            if ( ext_f )
                ok = fflush( fh ) != -1; // flush instead of closing
            else
                ok = fclose( fh ) != -1;
        } else {    // raw file
            if ( ext_f )
                ok = TRUE;   // cannot close
            else
                ok = QT_CLOSE( fd ) != -1;
        }
        init();     // restore internal state
    }
    if ( !ok ) {
        setStatus( IO_UnspecifiedError );
        setErrorStringErrno( errno );
    }
}
