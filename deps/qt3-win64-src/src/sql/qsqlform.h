/****************************************************************************
**
** Definition of QSqlForm class
**
** Created : 2000-11-03
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

#ifndef QSQLFORM_H
#define QSQLFORM_H

#ifndef QT_H
#include "qobject.h"
#include "qmap.h"
#endif // QT_H

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_EXPORT
#endif

#ifndef QT_NO_SQL_FORM

class QSqlField;
class QSqlRecord;
class QSqlEditorFactory;
class QSqlPropertyMap;
class QWidget;
class QSqlFormPrivate;

class QM_EXPORT_SQL QSqlForm : public QObject
{
    Q_OBJECT
public:
    QSqlForm( QObject * parent = 0, const char * name = 0 );
    ~QSqlForm();

    virtual void insert( QWidget * widget, const QString& field );
    virtual void remove( const QString& field );
    uint         count() const;

    QWidget *   widget( uint i ) const;
    QSqlField * widgetToField( QWidget * widget ) const;
    QWidget *   fieldToWidget( QSqlField * field ) const;

    void        installPropertyMap( QSqlPropertyMap * map );

    virtual void setRecord( QSqlRecord* buf );

public slots:
    virtual void readField( QWidget * widget );
    virtual void writeField( QWidget * widget );
    virtual void readFields();
    virtual void writeFields();

    virtual void clear();
    virtual void clearValues( bool nullify = FALSE );

protected:
    virtual void insert( QWidget * widget, QSqlField * field );
    virtual void remove( QWidget * widget );
    void clearMap();

private:
    virtual void sync();
    QSqlFormPrivate* d;

#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QSqlForm( const QSqlForm & );
    QSqlForm &operator=( const QSqlForm & );
#endif
};

#endif // QT_NO_SQL
#endif // QSQLFORM_H
