/****************************************************************************
**
** Definition of QSqlPropertyMap class
**
** Created : 2000-11-20
**
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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

#ifndef QSQLPROPERTYMAP_H
#define QSQLPROPERTYMAP_H

#ifndef QT_H
#include "qvariant.h"
#include "qstring.h"
#endif // QT_H

#ifndef QT_NO_SQL_FORM

class QWidget;
class QSqlPropertyMapPrivate;

class Q_EXPORT QSqlPropertyMap {
public:
    QSqlPropertyMap();
    virtual ~QSqlPropertyMap();

    QVariant      property( QWidget * widget );
    virtual void  setProperty( QWidget * widget, const QVariant & value );

    void insert( const QString & classname, const QString & property );
    void remove( const QString & classname );

    static QSqlPropertyMap * defaultMap();
    static void installDefaultMap( QSqlPropertyMap * map );

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSqlPropertyMap( const QSqlPropertyMap & );
    QSqlPropertyMap &operator=( const QSqlPropertyMap & );
#endif
    QSqlPropertyMapPrivate* d;

};

#endif // QT_NO_SQL_FORM
#endif // QSQLPROPERTYMAP_H
