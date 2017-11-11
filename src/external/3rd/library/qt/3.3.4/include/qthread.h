/****************************************************************************
** $Id: qt/qthread.h   3.3.4   edited May 27 2003 $
**
** Definition of QThread class
**
** Created : 931107
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
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

#ifndef QTHREAD_H
#define QTHREAD_H

#if defined(QT_THREAD_SUPPORT)

#ifndef QT_H
#include "qwindowdefs.h"
#ifndef QT_NO_COMPAT
#include "qmutex.h"
#include "qsemaphore.h"
#include "qwaitcondition.h"
#endif // QT_NO_COMPAT
#endif // QT_H

#include <limits.h>

class QThreadInstance;

class Q_EXPORT QThread : public Qt
{
public:
    static Qt::HANDLE currentThread();

#ifndef QT_NO_COMPAT
    static void postEvent( QObject *,QEvent * );
#endif

    static void initialize();
    static void cleanup();

    static void exit();

#ifdef Q_QDOC
    QThread( unsigned int stackSize = 0 );
#else
    QThread( unsigned int stackSize );
    QThread();
#endif

    virtual ~QThread();

    // default argument causes thread to block indefinately
    bool wait( unsigned long time = ULONG_MAX );

    enum Priority {
	IdlePriority,

	LowestPriority,
	LowPriority,
	NormalPriority,
	HighPriority,
	HighestPriority,

	TimeCriticalPriority,

	InheritPriority
    };

#ifdef Q_QDOC
    void start( Priority = InheritPriority );
#else
    void start( Priority );
    void start();
#endif

    void terminate();

    bool finished() const;
    bool running() const;

protected:
    virtual void run() = 0;

    static void sleep( unsigned long );
    static void msleep( unsigned long );
    static void usleep( unsigned long );

private:
    QThreadInstance * d;
    friend class QThreadInstance;

#if defined(Q_DISABLE_COPY)
    QThread( const QThread & );
    QThread &operator=( const QThread & );
#endif // Q_DISABLE_COPY
};

#endif // QT_THREAD_SUPPORT

#endif // QTHREAD_H
