/****************************************************************************
** $Id: qimageformatplugin.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** ...
**
** Copyright (C) 2001-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#include "qimageformatplugin.h"
#ifndef QT_NO_IMAGEFORMATPLUGIN
#include "qimageformatinterface_p.h"
#include "qimage.h"

/*!
    \class QImageFormatPlugin qimageformatplugin.h
    \brief The QImageFormatPlugin class provides an abstract base for custom image format plugins.

    \ingroup plugins

    The image format plugin is a simple plugin interface that makes
    it easy to create custom image formats that can be used
    transparently by applications.

    Writing an image format plugin is achieved by subclassing this
    base class, reimplementing the pure virtual functions keys() and
    installIOHandler(), and exporting the class with the
    Q_EXPORT_PLUGIN macro.  See the \link plugins-howto.html Plugins
    documentation\endlink for details.
*/

/*!
    \fn QStringList QImageFormatPlugin::keys() const

    Returns the list of image formats this plugin supports.

    \sa installIOHandler()
*/


/*!
    \fn  bool QImageFormatPlugin::installIOHandler( const QString &format )

    Installs a QImageIO image I/O handler for the image format \a
    format.

    \sa keys()
*/

class QImageFormatPluginPrivate : public QImageFormatInterface
{
public:
    QImageFormatPluginPrivate( QImageFormatPlugin *p )
	: plugin( p )
    {
    }
    virtual ~QImageFormatPluginPrivate();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;

    QRESULT loadImage( const QString &format, const QString &filename, QImage * );
    QRESULT saveImage( const QString &format, const QString &filename, const QImage & );

    QRESULT installIOHandler( const QString & );

private:
    QImageFormatPlugin *plugin;
};

QImageFormatPluginPrivate::~QImageFormatPluginPrivate()
{
    delete plugin;
}

QRESULT QImageFormatPluginPrivate::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = this;
    else if ( iid == IID_QFeatureList )
	*iface = this;
    else if ( iid == IID_QImageFormat )
	*iface = this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QStringList QImageFormatPluginPrivate::featureList() const
{
    return plugin->keys();
}

QRESULT QImageFormatPluginPrivate::loadImage( const QString &format, const QString &filename, QImage *image )
{
    return plugin->loadImage( format, filename, image ) ? QS_FALSE : QS_OK;
}

QRESULT QImageFormatPluginPrivate::saveImage( const QString &format, const QString &filename, const QImage &image )
{
    return plugin->saveImage( format, filename, image ) ? QS_FALSE : QS_OK;
}

QRESULT QImageFormatPluginPrivate::installIOHandler( const QString &format )
{
    return plugin->installIOHandler( format ) ? QS_FALSE : QS_OK;
}

/*!
    Constructs an image format plugin. This is invoked automatically
    by the Q_EXPORT_PLUGIN macro.
*/
QImageFormatPlugin::QImageFormatPlugin()
    : QGPlugin( d = new QImageFormatPluginPrivate( this ) )
{
}

/*!
    Destroys the image format plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QImageFormatPlugin::~QImageFormatPlugin()
{
}


/*!\internal
 */
bool QImageFormatPlugin::loadImage( const QString &format, const QString &filename, QImage *image )
{
    Q_UNUSED( format )
    Q_UNUSED( filename )
    Q_UNUSED( image )
    return FALSE;
}

/*! \internal
 */
bool QImageFormatPlugin::saveImage( const QString &format, const QString &filename, const QImage &image )
{
    Q_UNUSED( format )
    Q_UNUSED( filename )
    Q_UNUSED( image )
    return FALSE;
}

#endif // QT_NO_IMAGEFORMATPLUGIN
