/****************************************************************************
** $Id: qt/qwidgetlist.h   3.3.4   edited May 27 2003 $
**
** Definition of QWidgetList
**
** Created : 950116
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

#ifndef QWIDGETLIST_H
#define QWIDGETLIST_H

#ifndef QT_H
#include "qwidget.h"
#include "qptrlist.h"
#endif // QT_H

class Q_EXPORT QWidgetList : public QPtrList<QWidget>
{
public:
    QWidgetList() : QPtrList<QWidget>() {}
    QWidgetList( const QWidgetList &list ) : QPtrList<QWidget>(list) {}
   ~QWidgetList() { clear(); }
    QWidgetList &operator=(const QWidgetList &list)
	{ return (QWidgetList&)QPtrList<QWidget>::operator=(list); }
};

class Q_EXPORT QWidgetListIt : public QPtrListIterator<QWidget>
{
public:
    QWidgetListIt( const QWidgetList &l ) : QPtrListIterator<QWidget>(l) {}
    QWidgetListIt &operator=(const QWidgetListIt &i)
	{ return (QWidgetListIt&)QPtrListIterator<QWidget>::operator=(i); }
};

#endif // QWIDGETLIST_H
