/****************************************************************************
** $Id: qstylefactory.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QStyleFactory class
**
** Created : 001103
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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

#include "qstyleinterface_p.h" // up here for GCC 2.7.* compatibility
#include "qstylefactory.h"

#ifndef QT_NO_STYLE

#include "qapplication.h"
#include "qwindowsstyle.h"
#include "qmotifstyle.h"
#include "qcdestyle.h"
#include "qmotifplusstyle.h"
#include "qplatinumstyle.h"
#include "qsgistyle.h"
#include "qcompactstyle.h"
#ifndef QT_NO_STYLE_WINDOWSXP
#include "qwindowsxpstyle.h"
#endif
#ifndef QT_NO_STYLE_AQUA
#include "qaquastyle.h"
#endif
#ifndef QT_NO_STYLE_POCKETPC
#include "qpocketpcstyle_wce.h"
#endif

#if !defined( QT_NO_STYLE_MAC ) && defined( Q_WS_MAC )
QString p2qstring(const unsigned char *c); //qglobal.cpp
#include "qt_mac.h"
#include "qmacstyle_mac.h"
#endif
#include <stdlib.h>

#include <private/qpluginmanager_p.h>
#ifndef QT_NO_COMPONENT
class QStyleFactoryPrivate : public QObject
{
public:
    QStyleFactoryPrivate();
    ~QStyleFactoryPrivate();

    static QPluginManager<QStyleFactoryInterface> *manager;
};

static QStyleFactoryPrivate *instance = 0;
QPluginManager<QStyleFactoryInterface> *QStyleFactoryPrivate::manager = 0;

QStyleFactoryPrivate::QStyleFactoryPrivate()
: QObject( qApp )
{
    manager = new QPluginManager<QStyleFactoryInterface>( IID_QStyleFactory, QApplication::libraryPaths(), "/styles", FALSE );
}

QStyleFactoryPrivate::~QStyleFactoryPrivate()
{
    delete manager;
    manager = 0;

    instance = 0;
}

#endif //QT_NO_COMPONENT

/*!
    \class QStyleFactory qstylefactory.h
    \brief The QStyleFactory class creates QStyle objects.

    The style factory creates a QStyle object for a given key with
    QStyleFactory::create(key).

    The styles are either built-in or dynamically loaded from a style
    plugin (see \l QStylePlugin).

    QStyleFactory::keys() returns a list of valid keys, typically
    including "Windows", "Motif", "CDE", "MotifPlus", "Platinum",
    "SGI" and "Compact". Depending on the platform, "WindowsXP",
    "Aqua" or "Macintosh" may be available.
*/

/*!
    Creates a QStyle object that matches \a key case-insensitively.
    This is either a built-in style, or a style from a style plugin.

    \sa keys()
*/
QStyle *QStyleFactory::create( const QString& key )
{
    QStyle *ret = 0;
    QString style = key.lower();
#ifndef QT_NO_STYLE_WINDOWS
    if ( style == "windows" )
        ret = new QWindowsStyle;
    else
#endif
#ifndef QT_NO_STYLE_WINDOWSXP
    if ( style == "windowsxp" )
	ret = new QWindowsXPStyle;
    else
#endif
#ifndef QT_NO_STYLE_MOTIF
    if ( style == "motif" )
        ret = new QMotifStyle;
    else
#endif
#ifndef QT_NO_STYLE_CDE
    if ( style == "cde" )
        ret = new QCDEStyle;
    else
#endif
#ifndef QT_NO_STYLE_MOTIFPLUS
    if ( style == "motifplus" )
        ret = new QMotifPlusStyle;
    else
#endif
#ifndef QT_NO_STYLE_PLATINUM
    if ( style == "platinum" )
        ret = new QPlatinumStyle;
    else
#endif
#ifndef QT_NO_STYLE_SGI
    if ( style == "sgi")
        ret = new QSGIStyle;
    else
#endif
#ifndef QT_NO_STYLE_COMPACT
    if ( style == "compact" )
        ret = new QCompactStyle;
    else
#endif
#ifndef QT_NO_STYLE_AQUA
    if ( style == "aqua" )
        ret = new QAquaStyle;
#endif
#ifndef QT_NO_STYLE_POCKETPC
    if ( style == "pocketpc" )
	ret = new QPocketPCStyle;
#endif
#if !defined( QT_NO_STYLE_MAC ) && defined( Q_WS_MAC )
    if( style.left(9) == "macintosh" )
	ret = new QMacStyle;
#endif
    { } // Keep these here - they make the #ifdefery above work

#ifndef QT_NO_COMPONENT
    if(!ret) {
	if ( !instance )
	    instance = new QStyleFactoryPrivate;

	QInterfacePtr<QStyleFactoryInterface> iface;
	QStyleFactoryPrivate::manager->queryInterface( style, &iface );

	if ( iface )
	    ret = iface->create( style );
    }
    if(ret)
	ret->setName(key);
#endif
    return ret;
}

#ifndef QT_NO_STRINGLIST
/*!
    Returns the list of keys this factory can create styles for.

    \sa create()
*/
QStringList QStyleFactory::keys()
{
    QStringList list;
#ifndef QT_NO_COMPONENT
    if ( !instance )
	instance = new QStyleFactoryPrivate;

    list = QStyleFactoryPrivate::manager->featureList();
#endif //QT_NO_COMPONENT

#ifndef QT_NO_STYLE_WINDOWS
    if ( !list.contains( "Windows" ) )
	list << "Windows";
#endif
#ifndef QT_NO_STYLE_WINDOWSXP
    if ( !list.contains( "WindowsXP" ) && QWindowsXPStyle::resolveSymbols() )
	list << "WindowsXP";
#endif
#ifndef QT_NO_STYLE_MOTIF
    if ( !list.contains( "Motif" ) )
	list << "Motif";
#endif
#ifndef QT_NO_STYLE_CDE
    if ( !list.contains( "CDE" ) )
	list << "CDE";
#endif
#ifndef QT_NO_STYLE_MOTIFPLUS
    if ( !list.contains( "MotifPlus" ) )
	list << "MotifPlus";
#endif
#ifndef QT_NO_STYLE_PLATINUM
    if ( !list.contains( "Platinum" ) )
	list << "Platinum";
#endif
#ifndef QT_NO_STYLE_SGI
    if ( !list.contains( "SGI" ) )
	list << "SGI";
#endif
#ifndef QT_NO_STYLE_COMPACT
    if ( !list.contains( "Compact" ) )
	list << "Compact";
#endif
#ifndef QT_NO_STYLE_AQUA
    if ( !list.contains( "Aqua" ) )
	list << "Aqua";
#endif
#if !defined( QT_NO_STYLE_MAC ) && defined( Q_WS_MAC )
    QString mstyle = "Macintosh";
    Collection c = NewCollection();
    if (c) {
	GetTheme(c);
	Str255 str;
	long int s = 256;
	if(!GetCollectionItem(c, kThemeNameTag, 0, &s, &str))
	    mstyle += " (" + p2qstring(str) + ")";
    }
    if (!list.contains(mstyle))
	list << mstyle;
    DisposeCollection(c);
#endif

    return list;
}
#endif
#endif // QT_NO_STYLE
