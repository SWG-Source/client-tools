/****************************************************************************
**
** Definition of QDataBrowser class
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

#ifndef QDATABROWSER_H
#define QDATABROWSER_H

#ifndef QT_H
#include "qwidget.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qsql.h"
#include "qsqlindex.h"
#include "qsqlcursor.h"
#include "qsqlerror.h"
#endif // QT_H

#ifndef QT_NO_SQL_VIEW_WIDGETS

class QSqlForm;
class QDataBrowserPrivate;

class Q_EXPORT QDataBrowser : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( bool boundaryChecking READ boundaryChecking WRITE setBoundaryChecking )
    Q_PROPERTY( QString filter READ filter WRITE setFilter )
    Q_PROPERTY( QStringList sort READ sort WRITE setSort )
    Q_PROPERTY( bool confirmEdits READ confirmEdits WRITE setConfirmEdits )
    Q_PROPERTY( bool confirmInsert READ confirmInsert WRITE setConfirmInsert )
    Q_PROPERTY( bool confirmUpdate READ confirmUpdate WRITE setConfirmUpdate )
    Q_PROPERTY( bool confirmDelete READ confirmDelete WRITE setConfirmDelete )
    Q_PROPERTY( bool confirmCancels READ confirmCancels WRITE setConfirmCancels )
    Q_PROPERTY( bool readOnly READ isReadOnly WRITE setReadOnly )
    Q_PROPERTY( bool autoEdit READ autoEdit WRITE setAutoEdit )

public:
    QDataBrowser( QWidget* parent=0, const char* name=0, WFlags fl = 0 );
    ~QDataBrowser();

    enum Boundary {
	Unknown,
	None,
	BeforeBeginning,
	Beginning,
	End,
	AfterEnd
    };

    Boundary boundary();
    void setBoundaryChecking( bool active );
    bool boundaryChecking() const;

    void setSort( const QSqlIndex& sort );
    void setSort( const QStringList& sort );
    QStringList  sort() const;
    void setFilter( const QString& filter );
    QString filter() const;
    virtual void setSqlCursor( QSqlCursor* cursor, bool autoDelete = FALSE );
    QSqlCursor* sqlCursor() const;
    virtual void setForm( QSqlForm* form );
    QSqlForm* form();

    virtual void setConfirmEdits( bool confirm );
    virtual void setConfirmInsert( bool confirm );
    virtual void setConfirmUpdate( bool confirm );
    virtual void setConfirmDelete( bool confirm );
    virtual void setConfirmCancels( bool confirm );
    bool confirmEdits() const;
    bool confirmInsert() const;
    bool confirmUpdate() const;
    bool confirmDelete() const;
    bool confirmCancels() const;

    virtual void setReadOnly( bool active );
    bool isReadOnly() const;
    virtual void setAutoEdit( bool autoEdit );
    bool autoEdit() const;

    virtual bool seek( int i, bool relative = FALSE );

signals:
    void firstRecordAvailable( bool available );
    void lastRecordAvailable( bool available );
    void nextRecordAvailable( bool available );
    void prevRecordAvailable( bool available );

    void currentChanged( const QSqlRecord* record );
    void primeInsert( QSqlRecord* buf );
    void primeUpdate( QSqlRecord* buf );
    void primeDelete( QSqlRecord* buf );
    void beforeInsert( QSqlRecord* buf );
    void beforeUpdate( QSqlRecord* buf );
    void beforeDelete( QSqlRecord* buf );
    void cursorChanged( QSqlCursor::Mode mode );

public slots:
    virtual void refresh();

    virtual void insert();
    virtual void update();
    virtual void del();

    virtual void first();
    virtual void last();
    virtual void next();
    virtual void prev();

    virtual void readFields();
    virtual void writeFields();
    virtual void clearValues();

    void updateBoundary();

protected:
    virtual bool insertCurrent();
    virtual bool updateCurrent();
    virtual bool deleteCurrent();
    virtual bool currentEdited();

    virtual QSql::Confirm confirmEdit( QSql::Op m );
    virtual QSql::Confirm confirmCancel( QSql::Op m );

    virtual void handleError( const QSqlError& error );

private:
    typedef bool (QSqlCursor::*Nav)();
    bool preNav();
    void postNav( bool primeUpd );
    void nav( Nav nav );
    QDataBrowserPrivate* d;

#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QDataBrowser( const QDataBrowser & );
    QDataBrowser &operator=( const QDataBrowser & );
#endif
};


#endif
#endif
