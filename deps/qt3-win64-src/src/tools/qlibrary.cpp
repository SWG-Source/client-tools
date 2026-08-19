/****************************************************************************
** $Id: qlibrary.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QLibrary class
**
** Created : 000101
**
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech ASA of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qplatformdefs.h"
#include <private/qlibrary_p.h>
#include <qstringlist.h>
#include <qfile.h>

#ifndef QT_NO_LIBRARY

// uncomment this to get error messages
//#define QT_DEBUG_COMPONENT 1
// uncomment this to get error and success messages
//#define QT_DEBUG_COMPONENT 2

#ifndef QT_DEBUG_COMPONENT
# if defined(QT_DEBUG)
#  define QT_DEBUG_COMPONENT 1
# endif
#endif

#if (defined(Q_WS_WIN) && !defined(QT_MAKEDLL)) \
    || (defined(Q_OS_FREEBSD) && defined(Q_CC_INTEL)) \
    || (defined(Q_OS_IRIX) && defined(Q_CC_GNU))
#define QT_NO_LIBRARY_UNLOAD
#endif

QLibraryPrivate::QLibraryPrivate( QLibrary *lib )
    : pHnd( 0 ), library( lib )
{
}


/*!
    \class QLibrary qlibrary.h
    \reentrant
    \brief The QLibrary class provides a wrapper for handling shared libraries.

    \mainclass
    \ingroup plugins

    An instance of a QLibrary object can handle a single shared
    library and provide access to the functionality in the library in
    a platform independent way. If the library is a component server,
    QLibrary provides access to the exported component and can
    directly query this component for interfaces.

    QLibrary ensures that the shared library is loaded and stays in
    memory whilst it is in use. QLibrary can also unload the library
    on destruction and release unused resources.

    A typical use of QLibrary is to resolve an exported symbol in a
    shared object, and to call the function that this symbol
    represents. This is called "explicit linking" in contrast to
    "implicit linking", which is done by the link step in the build
    process when linking an executable against a library.

    The following code snippet loads a library, resolves the symbol
    "mysymbol", and calls the function if everything succeeded. If
    something went wrong, e.g. the library file does not exist or the
    symbol is not defined, the function pointer will be 0 and won't be
    called. When the QLibrary object is destroyed the library will be
    unloaded, making all references to memory allocated in the library
    invalid.

    \code
    typedef void (*MyPrototype)();
    MyPrototype myFunction;

    QLibrary myLib( "mylib" );
    myFunction = (MyPrototype) myLib.resolve( "mysymbol" );
    if ( myFunction ) {
	myFunction();
    }
    \endcode
*/

/*!
    Creates a QLibrary object for the shared library \a filename. The
    library will be unloaded in the destructor.

    Note that \a filename does not need to include the (platform specific)
    file extension, so calling
    \code
    QLibrary lib( "mylib" );
    \endcode
    is equivalent to calling
    \code
    QLibrary lib( "mylib.dll" );
    \endcode
    on Windows, and
    \code
    QLibrary lib( "libmylib.so" );
    \endcode
    on Unix. Specifying the extension is not recommended, since
    doing so introduces a platform dependency.

    If \a filename does not include a path, the library loader will
    look for the file in the platform specific search paths.

    \sa load() unload(), setAutoUnload()
*/
QLibrary::QLibrary( const QString& filename )
    : libfile( filename ), aunload( TRUE )
{
    libfile.replace( '\\', '/' );
    d = new QLibraryPrivate( this );
}

/*!
    Deletes the QLibrary object.

    The library will be unloaded if autoUnload() is TRUE (the
    default), otherwise it stays in memory until the application
    exits.

    \sa unload(), setAutoUnload()
*/
QLibrary::~QLibrary()
{
    if ( autoUnload() )
	unload();

    delete d;
}

/*!
    Returns the address of the exported symbol \a symb. The library is
    loaded if necessary. The function returns 0 if the symbol could
    not be resolved or the library could not be loaded.

    \code
    typedef int (*avgProc)( int, int );

    avgProc avg = (avgProc) library->resolve( "avg" );
    if ( avg )
	return avg( 5, 8 );
    else
	return -1;
    \endcode

    The symbol must be exported as a C-function from the library. This
    requires the \c {extern "C"} notation if the library is compiled
    with a C++ compiler. On Windows you also have to explicitly export
    the function from the DLL using the \c {__declspec(dllexport)}
    compiler directive.

    \code
    extern "C" MY_EXPORT_MACRO int avg(int a, int b)
    {
	return (a + b) / 2;
    }
    \endcode

    with \c MY_EXPORT defined as

    \code
    #ifdef Q_WS_WIN
    # define MY_EXPORT __declspec(dllexport)
    #else
    # define MY_EXPORT
    #endif
    \endcode

    On Darwin and Mac OS X this function uses code from dlcompat, part of the
    OpenDarwin project.

    \legalese

    Copyright (c) 2002 Jorge Acereda and Peter O'Gorman

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
void *QLibrary::resolve( const char* symb )
{
    if ( !d->pHnd )
	load();
    if ( !d->pHnd )
	return 0;

    void *address = d->resolveSymbol( symb );

    return address;
}

/*!
    \overload

    Loads the library \a filename and returns the address of the
    exported symbol \a symb. Note that like the constructor, \a
    filename does not need to include the (platform specific) file
    extension. The library remains loaded until the process exits.

    The function returns 0 if the symbol could not be resolved or the
    library could not be loaded.

    This function is useful only if you want to resolve a single
    symbol, e.g. a function pointer from a specific library once:

    \code
    typedef void (*FunctionType)();
    static FunctionType *ptrFunction = 0;
    static bool triedResolve = FALSE;
    if ( !ptrFunction && !triedResolve )
	ptrFunction = QLibrary::resolve( "mylib", "mysymb" );

    if ( ptrFunction )
	ptrFunction();
    else
	...
    \endcode

    If you want to resolve multiple symbols, use a QLibrary object and
    call the non-static version of resolve().

    \sa resolve()
*/
void *QLibrary::resolve( const QString &filename, const char *symb )
{
    QLibrary lib( filename );
    lib.setAutoUnload( FALSE );
    return lib.resolve( symb );
}

/*!
    Returns TRUE if the library is loaded; otherwise returns FALSE.

    \sa unload()
*/
bool QLibrary::isLoaded() const
{
    return d->pHnd != 0;
}

/*!
    Loads the library. Since resolve() always calls this function
    before resolving any symbols it is not necessary to call it
    explicitly. In some situations you might want the library loaded
    in advance, in which case you would use this function.

    On Darwin and Mac OS X this function uses code from dlcompat, part of the
    OpenDarwin project.

    \legalese

    Copyright (c) 2002 Jorge Acereda and Peter O'Gorman

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
bool QLibrary::load()
{
    if (libfile.isEmpty())
        return FALSE;
    return d->loadLibrary();
}

/*!
    Unloads the library and returns TRUE if the library could be
    unloaded; otherwise returns FALSE.

    This function is called by the destructor if autoUnload() is
    enabled.

    \sa resolve()
*/
bool QLibrary::unload()
{
    if ( !d->pHnd )
	return TRUE;

#if !defined(QT_NO_LIBRARY_UNLOAD)
    if ( !d->freeLibrary() ) {
# if defined(QT_DEBUG_COMPONENT)
	qWarning( "%s could not be unloaded", (const char*) QFile::encodeName(library()) );
# endif
	return FALSE;
    }

# if defined(QT_DEBUG_COMPONENT) && QT_DEBUG_COMPONENT == 2
    qWarning( "%s has been unloaded", (const char*) QFile::encodeName(library()) );
# endif
    d->pHnd = 0;
#endif
    return TRUE;
}

/*!
    Returns TRUE if the library will be automatically unloaded when
    this wrapper object is destructed; otherwise returns FALSE. The
    default is TRUE.

    \sa setAutoUnload()
*/
bool QLibrary::autoUnload() const
{
    return (bool)aunload;
}

/*!
    If \a enabled is TRUE (the default), the wrapper object is set to
    automatically unload the library upon destruction. If \a enabled
    is FALSE, the wrapper object is not unloaded unless you explicitly
    call unload().

    \sa autoUnload()
*/
void QLibrary::setAutoUnload( bool enabled )
{
    aunload = enabled;
}

/*!
    Returns the filename of the shared library this QLibrary object
    handles, including the platform specific file extension.

    For example:
    \code
    QLibrary lib( "mylib" );
    QString str = lib.library();
    \endcode
    will set \e str to "mylib.dll" on Windows, and "libmylib.so" on Linux.
*/
QString QLibrary::library() const
{
    if ( libfile.isEmpty() )
	return libfile;

    QString filename = libfile;

#if defined(Q_WS_WIN)
    if ( filename.findRev( '.' ) <= filename.findRev( '/' ) )
	filename += ".dll";
#else
    QStringList filters = "";
#ifdef Q_OS_MACX
    filters << ".so";
    filters << ".bundle";
    filters << ".dylib"; //the last one is also the default one..
#elif defined(Q_OS_HPUX)
    filters << ".sl";
#else
    filters << ".so";
#endif
    for(QStringList::Iterator it = filters.begin(); TRUE; ) {
	QString filter = (*it);
	++it;

	if(QFile::exists(filename + filter)) {
	    filename += filter;
	    break;
	} else if(!filter.isEmpty()) {
	    QString tmpfilename = filename;
	    const int x = tmpfilename.findRev( "/" );
	    if ( x != -1 ) {
		QString path = tmpfilename.left( x + 1 );
		QString file = tmpfilename.right( tmpfilename.length() - x - 1 );
		tmpfilename = QString( "%1lib%2" ).arg( path ).arg( file );
	    } else {
		tmpfilename = QString( "lib%1" ).arg( filename );
	    }
	    tmpfilename += filter;
	    if(QFile::exists(tmpfilename) || it == filters.end()) {
		filename = tmpfilename;
		break;
	    }
	}
    }
#endif
    return filename;
}
#endif //QT_NO_LIBRARY
