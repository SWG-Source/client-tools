/****************************************************************************
** $Id: qt/qtextcodecplugin.h   3.3.4   edited May 27 2003 $
**
** Definition of QTextCodecPlugin class
**
** Created : 010920
**
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
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

#ifndef QTEXTCODECPLUGIN_H
#define QTEXTCODECPLUGIN_H

#ifndef QT_H
#include "qgplugin.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_TEXTCODECPLUGIN
class QTextCodec;
class QTextCodecPluginPrivate;

class Q_EXPORT QTextCodecPlugin : public QGPlugin
{
    Q_OBJECT
public:
    QTextCodecPlugin();
    ~QTextCodecPlugin();

    virtual QStringList names() const = 0;
    virtual QTextCodec *createForName( const QString &name ) = 0;
    
    virtual QValueList<int> mibEnums() const = 0;
    virtual QTextCodec *createForMib( int mib ) = 0;

private:
    QTextCodecPluginPrivate *d;
};
#endif // QT_NO_TEXTCODECPLUGIN
#endif // QTEXTCODECPLUGIN_H
