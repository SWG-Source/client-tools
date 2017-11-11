/****************************************************************************
** $Id: qt/qptrstack.h   3.3.4   edited May 27 2003 $
**
** Definition of QPtrStack pointer based template class
**
** Created : 920917
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
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

#ifndef QPTRSTACK_H
#define QPTRSTACK_H

#ifndef QT_H
#include "qglist.h"
#endif // QT_H

template<class type>
class QPtrStack : protected QGList
{
public:
    QPtrStack()				{ }
    QPtrStack( const QPtrStack<type> &s ) : QGList( s ) { }
    ~QPtrStack()			{ clear(); }
    QPtrStack<type> &operator=(const QPtrStack<type> &s)
			{ return (QPtrStack<type>&)QGList::operator=(s); }
    bool  autoDelete() const		{ return QPtrCollection::autoDelete(); }
    void  setAutoDelete( bool del )	{ QPtrCollection::setAutoDelete(del); }
    uint  count()   const		{ return QGList::count(); }
    bool  isEmpty() const		{ return QGList::count() == 0; }
    void  push( const type *d )		{ QGList::insertAt(0,Item(d)); }
    type *pop()				{ return (type *)QGList::takeFirst(); }
    bool  remove()			{ return QGList::removeFirst(); }
    void  clear()			{ QGList::clear(); }
    type *top()	    const		{ return (type *)QGList::cfirst(); }
	  operator type *() const	{ return (type *)QGList::cfirst(); }
    type *current() const		{ return (type *)QGList::cfirst(); }

#ifdef Q_QDOC
protected:
    virtual QDataStream& read( QDataStream&, QPtrCollection::Item& );
    virtual QDataStream& write( QDataStream&, QPtrCollection::Item ) const;
#endif

private:
    void  deleteItem( Item d );
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void QPtrStack<void>::deleteItem( QPtrCollection::Item )
{
}
#endif

template<class type> inline void QPtrStack<type>::deleteItem( QPtrCollection::Item d )
{
    if ( del_item ) delete (type *)d;
}

#ifndef QT_NO_COMPAT
#define QStack QPtrStack
#endif

#endif // QPTRSTACK_H
