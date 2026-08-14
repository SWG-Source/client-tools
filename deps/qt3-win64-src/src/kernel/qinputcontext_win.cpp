/****************************************************************************
** $Id: qinputcontext_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QInputContext class
**
** Created : 20030714
**
** Copyright (C) 2004 Peter Kuemmel
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

#include "qplatformdefs.h"
#include "qapplication.h"
#include "qwidget.h"
#include "qinputcontext_p.h"

#include <stdlib.h>
#include <limits.h>


#ifdef Q_WS_WIN
void QInputContext::init()
{}

void QInputContext::shutdown()
{}

void QInputContext::TranslateMessage( const MSG *msg )
{}

LRESULT QInputContext::DefWindowProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
    return LRESULT();
}

void QInputContext::setFont( const QWidget *w, const QFont & )
{}

void QInputContext::setFocusHint( int x, int y, int w, int h, const QWidget *widget )
{}

bool QInputContext::startComposition()
{
    return true;
}

bool QInputContext::endComposition( QWidget *fw )
{
    return true;
}

bool QInputContext::composition( LPARAM lparam )
{
    return true;
}

void QInputContext::accept( QWidget *fw )
{}

void QInputContext::enable( QWidget *, bool )
{}
#endif

#ifdef Q_WS_QWS
void QInputContext::translateIMEvent( QWSIMEvent *, QWidget * )
{}
void QInputContext::reset()
{}
// Privates
QWidget* QInputContext::focusWidget;
QString* QInputContext::composition;
#endif //Q_WS_QWS
