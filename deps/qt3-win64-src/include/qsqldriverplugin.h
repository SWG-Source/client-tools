/****************************************************************************
** $Id: qsqldriverplugin.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Definition of QSqlDriverPlugin class
**
** Created : 2001-09-20
**
** Copyright (C) 2001-2007 Trolltech ASA.  All rights reserved.
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

#ifndef QSQLDRIVERPLUGIN_H
#define QSQLDRIVERPLUGIN_H

#ifndef QT_H
#include "qgplugin.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_SQL
#ifndef QT_NO_COMPONENT

class QSqlDriver;
class QSqlDriverPluginPrivate;

class Q_EXPORT QSqlDriverPlugin : public QGPlugin
{
    Q_OBJECT
public:
    QSqlDriverPlugin();
    ~QSqlDriverPlugin();

    virtual QStringList keys() const = 0;
    virtual QSqlDriver *create( const QString &key ) = 0;

private:
    QSqlDriverPluginPrivate *d;
};

#endif // QT_NO_COMPONENT
#endif // QT_NO_SQL

#endif // QSQLDRIVERPLUGIN_H
