/****************************************************************************
** $Id: qt/qmotif.h   3.3.4   edited Nov 17 2003 $
**
** Definition of Qt extension classes for Xt/Motif support.
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt extension for Xt/Motif support.
**
** Licensees holding valid Qt Enterprise Edition licenses for X11 may use
** this file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QMOTIF_H
#define QMOTIF_H

#include <qeventloop.h>

#include <X11/Intrinsic.h>

class QMotifPrivate;

class QMotif : public QEventLoop
{
    Q_OBJECT

public:
    QMotif( const char *applicationClass, XtAppContext context = NULL, XrmOptionDescRec *options = 0, int numOptions = 0);
    ~QMotif();

    XtAppContext applicationContext() const;

    void registerSocketNotifier( QSocketNotifier * );
    void unregisterSocketNotifier( QSocketNotifier * );

    static void registerWidget( QWidget* );
    static void unregisterWidget( QWidget* );
    static bool redeliverEvent( XEvent *event );

    static Display *x11Display();
    static XEvent* lastEvent();

protected:
    bool processEvents( ProcessEventsFlags flags );

private:
    void appStartingUp();
    void appClosingDown();
    QMotifPrivate *d;
};

#endif // QMOTIF_H
