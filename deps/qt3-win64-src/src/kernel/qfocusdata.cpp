/****************************************************************************
** $Id: qfocusdata.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QFocusData class
**
** Created : 980622
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#include "qfocusdata.h"

/*!
    \class QFocusData qfocusdata.h
    \brief The QFocusData class maintains the list of widgets in the focus
    chain.

    \ingroup misc

    This read-only list always contains at least one widget (i.e. the
    top-level widget). It provides a simple cursor which can be reset
    to the current focus widget using home(), or moved to its
    neighboring widgets using next() and prev(). You can also retrieve
    the count() of the number of widgets in the list. The list is a
    loop, so if you keep iterating, for example using next(), you will
    never come to the end.

    Some widgets in the list may not accept focus. Widgets are added
    to the list as necessary, but not removed from it. This lets
    widgets change focus policy dynamically without disrupting the
    focus chain the user experiences. When a widget disables and
    re-enables tab focus, its position in the focus chain does not
    change.

    When reimplementing QWidget::focusNextPrevChild() to provide
    special focus flow, you will usually call QWidget::focusData() to
    retrieve the focus data stored at the top-level widget. A
    top-level widget's focus data contains the focus list for its
    hierarchy of widgets.

    The cursor may change at any time.

    This class is \e not thread-safe.

    \sa QWidget::focusNextPrevChild() QWidget::setTabOrder()
    QWidget::setFocusPolicy()
*/

/*!
    \fn QWidget* QFocusData::focusWidget() const

    Returns the widgets in the hierarchy that are in the focus chain.
*/

/*!
    \fn int QFocusData::count() const

    Returns the number of widgets in the focus chain.
*/

/*!
    Moves the cursor to the focusWidget() and returns that widget. You
    must call this before next() or prev() to iterate meaningfully.
*/
QWidget* QFocusData::home()
{
    focusWidgets.find(it.current());
    return focusWidgets.current();
}

/*!
    Moves the cursor to the next widget in the focus chain. There is
    \e always a next widget because the list is a loop.
*/
QWidget* QFocusData::next()
{
    QWidget* r = focusWidgets.next();
    if ( !r )
	r = focusWidgets.first();
    return r;
}

/*!
    Moves the cursor to the previous widget in the focus chain. There
    is \e always a previous widget because the list is a loop.
*/
QWidget* QFocusData::prev()
{
    QWidget* r = focusWidgets.prev();
    if ( !r )
	r = focusWidgets.last();
    return r;
}

/*!
    Returns the last widget in the focus chain.
    The cursor is not modified.
*/
QWidget *QFocusData::last() const
{
    return focusWidgets.getLast();
}

/*!
    Returns the first widget in the focus chain.
    The cursor is not modified.
*/
QWidget *QFocusData::first() const
{
    return focusWidgets.getFirst();
}
