/****************************************************************************
** $Id: qt/qobjectlist.h   3.3.4   edited May 27 2003 $
**
** Definition of QObjectList
**
** Created : 940807
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
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

#ifndef QOBJECTLIST_H
#define QOBJECTLIST_H

#ifndef QT_H
#include "qobject.h"
#include "qptrlist.h"
#endif // QT_H


#if defined(Q_TEMPLATEDLL)
//Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrList<QObject>;
//Q_TEMPLATE_EXTERN template class Q_EXPORT QPtrListIterator<QObject>;
#endif


class Q_EXPORT QObjectList : public QPtrList<QObject>
{
public:
    QObjectList() : QPtrList<QObject>() {}
    QObjectList( const QObjectList &list ) : QPtrList<QObject>(list) {}
   ~QObjectList() { clear(); }
    QObjectList &operator=(const QObjectList &list)
	{ return (QObjectList&)QPtrList<QObject>::operator=(list); }
};

class Q_EXPORT QObjectListIterator : public QPtrListIterator<QObject>
{
public:
    QObjectListIterator( const QObjectList &l )
	: QPtrListIterator<QObject>( l ) { }
    QObjectListIterator &operator=( const QObjectListIterator &i )
	{ return (QObjectListIterator&)
		 QPtrListIterator<QObject>::operator=( i ); }
};

#if (QT_VERSION-0 >= 0x040000)
#if defined(Q_CC_GNU)
#warning "remove the QObjectListIt class"
#warning "remove the typedef too, maybe"
#endif
typedef QObjectListIterator QObjectListIt;
#else
class Q_EXPORT QObjectListIt : public QPtrListIterator<QObject>
{
public:
    QObjectListIt( const QObjectList &l ) : QPtrListIterator<QObject>(l) {}
    QObjectListIt &operator=(const QObjectListIt &i)
	{ return (QObjectListIt&)QPtrListIterator<QObject>::operator=(i); }
};
#endif

#endif // QOBJECTLIST_H
