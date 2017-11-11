/****************************************************************************
** $Id: qt/qimageformatplugin.h   3.3.4   edited May 27 2003 $
**
** Definition of ???
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

#ifndef QIMAGEFORMATPLUGIN_H
#define QIMAGEFORMATPLUGIN_H

#ifndef QT_H
#include "qgplugin.h"
#include "qstringlist.h"
#endif // QT_H

#ifndef QT_NO_IMAGEFORMATPLUGIN
class QImageFormat;
class QImageFormatPluginPrivate;

class Q_EXPORT QImageFormatPlugin : public QGPlugin
{
    Q_OBJECT
public:
    QImageFormatPlugin();
    ~QImageFormatPlugin();

    virtual QStringList keys() const = 0;
    virtual bool loadImage( const QString &format, const QString &filename, QImage *image );
    virtual bool saveImage( const QString &format, const QString &filename, const QImage &image );
    virtual bool installIOHandler( const QString &format ) = 0;

private:
    QImageFormatPluginPrivate *d;
};
#endif // QT_NO_IMAGEFORMATPLUGIN
#endif // QIMAGEFORMATPLUGIN_H
