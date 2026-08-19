/****************************************************************************
**
** Copyright (C) 1992-2005 Trolltech AS. All rights reserved.
**
** This file is part of the Qt 3 compatibility classes of the Qt Toolkit.
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
#include "qprocess.h"

#ifndef QT_NO_PROCESS

#include "qapplication.h"
#include "qcstring.h"
#include "qptrqueue.h"
#include "qtimer.h"
#include "qregexp.h"
#include "qinternal_p.h"
#include "qt_windows.h"

//#define QT_QPROCESS_DEBUG

// qt4\src\corelib\global\qglobal.cpp
// getenv is declared as deprecated in VS2005. This function
// makes use of the new secure getenv function.
QByteArray qgetenv(const char *varName)
{
#if defined(_MSC_VER) && _MSC_VER >= 1400
    size_t requiredSize;
    QByteArray buffer;
    getenv_s(&requiredSize, 0, 0, varName);
    if (requiredSize == 0)
        return buffer;
    buffer.resize(requiredSize);
    getenv_s(&requiredSize, buffer.data(), requiredSize, varName);
    return buffer;
#else
    return QCString(::getenv(varName));
#endif
}
/***********************************************************************
 *
 * QProcessPrivate
 *
 **********************************************************************/
class QProcessPrivate
{
public:
    QProcessPrivate( QProcess *proc )
    {
	stdinBufRead = 0;
	pipeStdin[0] = 0;
	pipeStdin[1] = 0;
	pipeStdout[0] = 0;
	pipeStdout[1] = 0;
	pipeStderr[0] = 0;
	pipeStderr[1] = 0;
	exitValuesCalculated = false;

	lookup = new QTimer( proc );
	qApp->connect( lookup, SIGNAL(timeout()),
		proc, SLOT(timeout()) );

	pid = 0;
    }

    ~QProcessPrivate()
    {
	reset();
    }

    void reset()
    {
	while ( !stdinBuf.isEmpty() ) {
	    delete stdinBuf.dequeue();
	}
	closeHandles();
	stdinBufRead = 0;
	pipeStdin[0] = 0;
	pipeStdin[1] = 0;
	pipeStdout[0] = 0;
	pipeStdout[1] = 0;
	pipeStderr[0] = 0;
	pipeStderr[1] = 0;
	exitValuesCalculated = false;

	deletePid();
    }

    void closeHandles()
    {
	if( pipeStdin[1] != 0 ) {
	    CloseHandle( pipeStdin[1] );
	    pipeStdin[1] = 0;
	}
	if( pipeStdout[0] != 0 ) {
	    CloseHandle( pipeStdout[0] );
	    pipeStdout[0] = 0;
	}
	if( pipeStderr[0] != 0 ) {
	    CloseHandle( pipeStderr[0] );
	    pipeStderr[0] = 0;
	}
    }

    void deletePid()
    {
	if ( pid ) {
	    CloseHandle( pid->hProcess );
	    CloseHandle( pid->hThread );
	    delete pid;
	    pid = 0;
	}
    }

    void newPid()
    {
	deletePid();
	pid = new PROCESS_INFORMATION;
	memset( pid, 0, sizeof(PROCESS_INFORMATION) );
    }

    QMembuf bufStdout;
    QMembuf bufStderr;

    QPtrQueue<QByteArray> stdinBuf;

    HANDLE pipeStdin[2];
    HANDLE pipeStdout[2];
    HANDLE pipeStderr[2];
    QTimer *lookup;

    PROCESS_INFORMATION *pid;
    uint stdinBufRead;

    bool exitValuesCalculated;
};


/***********************************************************************
 *
 * QProcess
 *
 **********************************************************************/
void QProcess::init()
{
    d = new QProcessPrivate( this );
    exitStat = 0;
    exitNormal = false;
}

void QProcess::reset()
{
    d->reset();
    exitStat = 0;
    exitNormal = false;
    d->bufStdout.clear();
    d->bufStderr.clear();
}

QMembuf* QProcess::membufStdout()
{
    if( d->pipeStdout[0] != 0 )
	socketRead( 1 );
    return &d->bufStdout;
}

QMembuf* QProcess::membufStderr()
{
    if( d->pipeStderr[0] != 0 )
	socketRead( 2 );
    return &d->bufStderr;
}

QProcess::~QProcess()
{
    delete d;
}

bool QProcess::start( QStringList *env )
{
#if defined(QT_QProcess_DEBUG)
    qDebug( "QProcess::start()" );
#endif
    reset();

    if ( _arguments.isEmpty() )
	return false;

    // Open the pipes.  Make non-inheritable copies of input write and output
    // read handles to avoid non-closable handles (this is done by the
    // DuplicateHandle() call).
    SECURITY_ATTRIBUTES secAtt = { sizeof( SECURITY_ATTRIBUTES ), NULL, TRUE };
#ifndef Q_OS_TEMP
    // I guess there is no stdin stdout and stderr on Q_OS_TEMP to dup
    // CreatePipe and DupilcateHandle aren't avaliable for Q_OS_TEMP
    HANDLE tmpStdin, tmpStdout, tmpStderr;
    if ( comms & Stdin ) {
	if ( !CreatePipe( &d->pipeStdin[0], &tmpStdin, &secAtt, 0 ) ) {
	    d->closeHandles();
	    return false;
	}
	if ( !DuplicateHandle( GetCurrentProcess(), tmpStdin, GetCurrentProcess(), &d->pipeStdin[1], 0, FALSE, DUPLICATE_SAME_ACCESS ) ) {
	    d->closeHandles();
	    return false;
	}
	if ( !CloseHandle( tmpStdin ) ) {
	    d->closeHandles();
	    return false;
	}
    }
    if ( comms & Stdout ) {
	if ( !CreatePipe( &tmpStdout, &d->pipeStdout[1], &secAtt, 0 ) ) {
	    d->closeHandles();
	    return false;
	}
	if ( !DuplicateHandle( GetCurrentProcess(), tmpStdout, GetCurrentProcess(), &d->pipeStdout[0], 0, FALSE, DUPLICATE_SAME_ACCESS ) ) {
	    d->closeHandles();
	    return false;
	}
	if ( !CloseHandle( tmpStdout ) ) {
	    d->closeHandles();
	    return false;
	}
    }
    if ( comms & Stderr ) {
	if ( !CreatePipe( &tmpStderr, &d->pipeStderr[1], &secAtt, 0 ) ) {
	    d->closeHandles();
	    return false;
	}
	if ( !DuplicateHandle( GetCurrentProcess(), tmpStderr, GetCurrentProcess(), &d->pipeStderr[0], 0, FALSE, DUPLICATE_SAME_ACCESS ) ) {
	    d->closeHandles();
	    return false;
	}
	if ( !CloseHandle( tmpStderr ) ) {
	    d->closeHandles();
	    return false;
	}
    }
    if ( comms & DupStderr ) {
	CloseHandle( d->pipeStderr[1] );
	d->pipeStderr[1] = d->pipeStdout[1];
    }
#endif

    // construct the arguments for CreateProcess()
    QString args;
    QString appName;
    QStringList::Iterator it = _arguments.begin();
    args = *it;
    ++it;
    if ( args.endsWith( ".bat" ) && args.contains( ' ' ) ) {
	// CreateProcess() seems to have a strange semantics (see also
	// http://www.experts-exchange.com/Programming/Programming_Platforms/Win_Prog/Q_11138647.html):
	// If you start a batch file with spaces in the filename, the first
	// argument to CreateProcess() must be the name of the batchfile
	// without quotes, but the second argument must start with the same
	// argument with quotes included. But if the same approach is used for
	// .exe files, it doesn't work.
	appName = args;
	args = '"' + args + '"';
    }
    for ( ; it != _arguments.end(); ++it ) {
	QString tmp = *it;
	// escape a single " because the arguments will be parsed
	tmp.replace( "\"", "\\\"" );
	if ( tmp.isEmpty() || tmp.contains( ' ' ) || tmp.contains( '\t' ) ) {
	    // The argument must not end with a \ since this would be interpreted
	    // as escaping the quote -- rather put the \ behind the quote: e.g.
	    // rather use "foo"\ than "foo\"
	    QString endQuote( "\"" );
	    int i = tmp.length();
	    while ( i>=0 && tmp.at( i-1 ) == '\\' ) {
		--i;
		endQuote += "\\";
	    }
	    args += QString( " \"" ) + tmp.left( i ) + endQuote;
	} else {
	    args += ' ' + tmp;
	}
    }
#if defined(QT_QProcess_DEBUG)
    qDebug( "QProcess::start(): args [%s]", args.latin1() );
#endif

    // CreateProcess()
    bool success;
    d->newPid();
#ifdef UNICODE
    if (!(QApplication::winVersion() & Qt::WV_DOS_based)) {
	STARTUPINFOW startupInfo = {
	    sizeof( STARTUPINFO ), 0, 0, 0,
	    (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
	    0, 0, 0,
	    STARTF_USESTDHANDLES,
	    0, 0, 0,
	    d->pipeStdin[0], d->pipeStdout[1], d->pipeStderr[1]
	};
	TCHAR *applicationName;
	if ( appName.isNull() )
	    applicationName = 0;
	else
	    applicationName = _wcsdup( (TCHAR*)appName.ucs2() );
	TCHAR *commandLine = _wcsdup( (TCHAR*)args.ucs2() );
	QByteArray envlist;
	if ( env != 0 ) {
	    int pos = 0;
	    // add PATH if necessary (for DLL loading)
	    QByteArray path = qgetenv( "PATH" );
	    if ( env->grep( QRegExp("^PATH=",FALSE) ).empty() && !path.isNull() ) {
		QString tmp = QString( "PATH=%1" ).arg(QString(path));
		uint tmpSize = sizeof(TCHAR) * (tmp.length()+1);
		envlist.resize( envlist.size() + tmpSize );
		memcpy( envlist.data()+pos, tmp.ucs2(), tmpSize );
		pos += tmpSize;
	    }
	    // add the user environment
	    for ( QStringList::Iterator it = env->begin(); it != env->end(); it++ ) {
		QString tmp = *it;
		uint tmpSize = sizeof(TCHAR) * (tmp.length()+1);
		envlist.resize( envlist.size() + tmpSize );
		memcpy( envlist.data()+pos, tmp.ucs2(), tmpSize );
		pos += tmpSize;
	    }
	    // add the 2 terminating 0 (actually 4, just to be on the safe side)
	    envlist.resize( envlist.size()+4 );
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
	}
	success = CreateProcessW( applicationName, commandLine,
		0, 0, TRUE, ( comms==0 ? CREATE_NEW_CONSOLE : CREATE_NO_WINDOW )
#ifndef Q_OS_TEMP
		| CREATE_UNICODE_ENVIRONMENT
#endif
		, env==0 ? 0 : envlist.data(),
                (TCHAR*)QDir::convertSeparators(workingDir.absPath()).ucs2(),
		&startupInfo, d->pid );
	free( applicationName );
	free( commandLine );
    } else
#endif // UNICODE
    {
#ifndef Q_OS_TEMP
	STARTUPINFOA startupInfo = { sizeof( STARTUPINFOA ), 0, 0, 0,
	    (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
	    0, 0, 0,
	    STARTF_USESTDHANDLES,
	    0, 0, 0,
	    d->pipeStdin[0], d->pipeStdout[1], d->pipeStderr[1]
	};
	QByteArray envlist;
	if ( env != 0 ) {
	    int pos = 0;
	    // add PATH if necessary (for DLL loading)
	    QByteArray path = qgetenv( "PATH" );
	    if ( env->grep( QRegExp("^PATH=",FALSE) ).empty() && !path.isNull() ) {
		QCString tmp = QString( "PATH=%1" ).arg(QString(path)).local8Bit();
		uint tmpSize = tmp.length() + 1;
		envlist.resize( envlist.size() + tmpSize );
		memcpy( envlist.data()+pos, tmp.data(), tmpSize );
		pos += tmpSize;
	    }
	    // add the user environment
	    for ( QStringList::Iterator it = env->begin(); it != env->end(); it++ ) {
		QCString tmp = (*it).local8Bit();
		uint tmpSize = tmp.length() + 1;
		envlist.resize( envlist.size() + tmpSize );
		memcpy( envlist.data()+pos, tmp.data(), tmpSize );
		pos += tmpSize;
	    }
	    // add the terminating 0 (actually 2, just to be on the safe side)
	    envlist.resize( envlist.size()+2 );
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
	}
	char *applicationName;
	if ( appName.isNull() )
	    applicationName = 0;
	else
	    applicationName = const_cast<char *>(appName.local8Bit().data());
	success = CreateProcessA( applicationName,
		const_cast<char *>(args.local8Bit().data()),
		0, 0, TRUE, comms==0 ? CREATE_NEW_CONSOLE : DETACHED_PROCESS,
		env==0 ? 0 : envlist.data(),
                (const char*)QDir::convertSeparators(workingDir.absPath()).local8Bit(),
		&startupInfo, d->pid );
#endif // Q_OS_TEMP
    }
    if  ( !success ) {
	d->deletePid();
	return false;
    }

#ifndef Q_OS_TEMP
    if ( comms & Stdin )
	CloseHandle( d->pipeStdin[0] );
    if ( comms & Stdout )
        CloseHandle( d->pipeStdout[1] );
    if ( (comms & Stderr) && !(comms & DupStderr) )
	CloseHandle( d->pipeStderr[1] );
#endif

    if ( ioRedirection || notifyOnExit ) {
	d->lookup->start( 100 );
    }

    // cleanup and return
    return true;
}

static BOOL CALLBACK qt_terminateApp( HWND hwnd, LPARAM procId )
{
    DWORD procId_win;
    GetWindowThreadProcessId( hwnd, &procId_win );
    if( procId_win == (DWORD)procId )
	PostMessage( hwnd, WM_CLOSE, 0, 0 );

    return TRUE;
}

void QProcess::tryTerminate() const
{
    if ( d->pid )
	EnumWindows( qt_terminateApp, (LPARAM)d->pid->dwProcessId );
}

void QProcess::kill() const
{
    if ( d->pid )
	TerminateProcess( d->pid->hProcess, 0xf291 );
}

bool QProcess::isRunning() const
{
    if ( !d->pid )
	return false;

    if ( WaitForSingleObject( d->pid->hProcess, 0) == WAIT_OBJECT_0 ) {
	// there might be data to read
	QProcess *that = (QProcess*)this;
	that->socketRead( 1 ); // try stdout
	that->socketRead( 2 ); // try stderr
	// compute the exit values
	if ( !d->exitValuesCalculated ) {
	    DWORD exitCode;
	    if ( GetExitCodeProcess( d->pid->hProcess, &exitCode ) ) {
		if ( exitCode != STILL_ACTIVE ) { // this should ever be true?
		    that->exitNormal = exitCode != 0xf291;
		    that->exitStat = exitCode;
		}
	    }
	    d->exitValuesCalculated = true;
	}
	d->deletePid();
	d->closeHandles();
	return false;
    } else {
        return true;
    }
}

bool QProcess::canReadLineStdout() const
{
    if( !d->pipeStdout[0] )
	return d->bufStdout.size() != 0;

    QProcess *that = (QProcess*)this;
    return that->membufStdout()->scanNewline( 0 );
}

bool QProcess::canReadLineStderr() const
{
    if( !d->pipeStderr[0] )
	return d->bufStderr.size() != 0;

    QProcess *that = (QProcess*)this;
    return that->membufStderr()->scanNewline( 0 );
}

void QProcess::writeToStdin( const QByteArray& buf )
{
    d->stdinBuf.enqueue( new QByteArray(buf) );
    socketWrite( 0 );
}

void QProcess::closeStdin( )
{
    if ( d->pipeStdin[1] != 0 ) {
	CloseHandle( d->pipeStdin[1] );
	d->pipeStdin[1] = 0;
    }
}

void QProcess::socketRead( int fd )
{
    // fd == 1: stdout, fd == 2: stderr
    HANDLE dev;
    if ( fd == 1 ) {
	dev = d->pipeStdout[0];
    } else if ( fd == 2 ) {
	dev = d->pipeStderr[0];
    } else {
	return;
    }
#ifndef Q_OS_TEMP
    // get the number of bytes that are waiting to be read
    unsigned long i, r;
    char dummy;
    if ( !PeekNamedPipe( dev, &dummy, 1, &r, &i, 0 ) ) {
	return; // ### is it worth to dig for the reason of the error?
    }
#else
    unsigned long i = 1000;
#endif
    if ( i > 0 ) {
	QMembuf *buffer;
	if ( fd == 1 )
	    buffer = &d->bufStdout;
	else
	    buffer = &d->bufStderr;

	QByteArray *ba = new QByteArray( i );
	uint sz = readStddev( dev, ba->data(), i );
	if ( sz != i )
	    ba->resize( i );

	if ( sz == 0 ) {
	    delete ba;
	    return;
	}
	buffer->append( ba );
	if ( fd == 1 )
	    emit readyReadStdout();
	else
	    emit readyReadStderr();
    }
}

void QProcess::socketWrite( int )
{
    DWORD written;
    while ( !d->stdinBuf.isEmpty() && isRunning() ) {
	if ( !WriteFile( d->pipeStdin[1],
		    d->stdinBuf.head()->data() + d->stdinBufRead,
		    QMIN( 8192, int(d->stdinBuf.head()->size() - d->stdinBufRead) ),
		    &written, 0 ) ) {
	    d->lookup->start( 100 );
	    return;
	}
	d->stdinBufRead += written;
	if ( d->stdinBufRead == (DWORD)d->stdinBuf.head()->size() ) {
	    d->stdinBufRead = 0;
	    delete d->stdinBuf.dequeue();
	    if ( wroteToStdinConnected && d->stdinBuf.isEmpty() )
		emit wroteToStdin();
	}
    }
}

void QProcess::flushStdin()
{
    socketWrite( 0 );
}

/*
  Use a timer for polling misc. stuff.
*/
void QProcess::timeout()
{
    // Disable the timer temporary since one of the slots that are connected to
    // the readyRead...(), etc. signals might trigger recursion if
    // processEvents() is called.
    d->lookup->stop();

    // try to write pending data to stdin
    if ( !d->stdinBuf.isEmpty() )
	socketWrite( 0 );

    if ( ioRedirection ) {
	socketRead( 1 ); // try stdout
	socketRead( 2 ); // try stderr
    }

    if ( isRunning() ) {
	// enable timer again, if needed
	if ( !d->stdinBuf.isEmpty() || ioRedirection || notifyOnExit )
	    d->lookup->start( 100 );
    } else if ( notifyOnExit ) {
	emit processExited();
    }
}

/*
  read on the pipe
*/
uint QProcess::readStddev( HANDLE dev, char *buf, uint bytes )
{
    if ( bytes > 0 ) {
	ulong r;
	if ( ReadFile( dev, buf, bytes, &r, 0 ) )
	    return r;
    }
    return 0;
}

/*
  Used by connectNotify() and disconnectNotify() to change the value of
  ioRedirection (and related behaviour)
*/
void QProcess::setIoRedirection( bool value )
{
    ioRedirection = value;
    if ( !ioRedirection && !notifyOnExit )
	d->lookup->stop();
    if ( ioRedirection ) {
	if ( isRunning() )
	    d->lookup->start( 100 );
    }
}

/*
  Used by connectNotify() and disconnectNotify() to change the value of
  notifyOnExit (and related behaviour)
*/
void QProcess::setNotifyOnExit( bool value )
{
    notifyOnExit = value;
    if ( !ioRedirection && !notifyOnExit )
	d->lookup->stop();
    if ( notifyOnExit ) {
	if ( isRunning() )
	    d->lookup->start( 100 );
    }
}

/*
  Used by connectNotify() and disconnectNotify() to change the value of
  wroteToStdinConnected (and related behaviour)
*/
void QProcess::setWroteStdinConnected( bool value )
{
    wroteToStdinConnected = value;
}

QProcess::PID QProcess::processIdentifier()
{
    return d->pid;
}

#endif // QT_NO_PROCESS
