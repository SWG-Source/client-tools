/****************************************************************************
** $Id: qhbuttongroup.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QHButtonGroup class
**
** Created : 990602
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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

#include "qhbuttongroup.h"
#ifndef QT_NO_HBUTTONGROUP

/*!
    \class QHButtonGroup qhbuttongroup.h
    \brief The QHButtonGroup widget organizes QButton widgets in a
    group with one horizontal row.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance

    QHButtonGroup is a convenience class that offers a thin layer on
    top of QButtonGroup. From a layout point of view it is effectively
    a QHBox that offers a frame with a title and is specifically
    designed for buttons. From a functionality point of view it is a
    QButtonGroup.

    \img qbuttongroup-h.png QButtonGroup

    \sa QVButtonGroup
*/

/*!
    Constructs a horizontal button group with no title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/
QHButtonGroup::QHButtonGroup( QWidget *parent, const char *name )
    : QButtonGroup( 1, Vertical /* sic! */, parent, name )
{
}

/*!
    Constructs a horizontal button group with the title \a title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

QHButtonGroup::QHButtonGroup( const QString &title, QWidget *parent,
			    const char *name )
    : QButtonGroup( 1, Vertical /* sic! */, title, parent, name )
{
}

/*!
    Destroys the horizontal button group, deleting its child widgets.
*/
QHButtonGroup::~QHButtonGroup()
{
}
#endif
