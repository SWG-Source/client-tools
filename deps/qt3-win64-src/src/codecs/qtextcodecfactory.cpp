/****************************************************************************
**
** Implementation of QTextCodecFactory class
**
** Created : 010130
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
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

#include "qtextcodecfactory.h"

#ifndef QT_NO_TEXTCODEC

#ifndef QT_NO_COMPONENT
#include "qapplication.h"
#include "qcleanuphandler.h"
#include <private/qpluginmanager_p.h>
#include "qtextcodecinterface_p.h"

#ifdef QT_THREAD_SUPPORT
#  include <private/qmutexpool_p.h>
#endif // QT_THREAD_SUPPORT

#include <stdlib.h>


static QPluginManager<QTextCodecFactoryInterface> *manager = 0;
static QSingleCleanupHandler< QPluginManager<QTextCodecFactoryInterface> > cleanup_manager;

static void create_manager()
{
    if ( manager ) // already created
	return;

#ifdef QT_THREAD_SUPPORT
    // protect manager creation
    QMutexLocker locker( qt_global_mutexpool ?
			 qt_global_mutexpool->get( &manager ) : 0);

    // we check the manager pointer again to make sure that another thread
    // has not created the manager before us.

    if ( manager ) // already created
	return;
#endif

    manager =
	new QPluginManager<QTextCodecFactoryInterface>(IID_QTextCodecFactory,
						       QApplication::libraryPaths(), "/codecs",
						       FALSE);
    Q_CHECK_PTR( manager );
    cleanup_manager.set( &manager );
}

#endif // QT_NO_COMPONENT


QTextCodec *QTextCodecFactory::createForName(const QString &name)
{
    QTextCodec *codec = 0;

#ifndef QT_NO_COMPONENT

    // make sure the manager is created
    create_manager();

    QInterfacePtr<QTextCodecFactoryInterface> iface;
    manager->queryInterface(name, &iface );

    if (iface)
	codec = iface->createForName(name);

#endif // QT_NO_COMPONENT

    return codec;
}


QTextCodec *QTextCodecFactory::createForMib(int mib)
{
    QTextCodec *codec = 0;

#ifndef QT_NO_COMPONENT

    // make sure the manager is created
    create_manager();

    QInterfacePtr<QTextCodecFactoryInterface> iface;
    manager->queryInterface("MIB-" + QString::number(mib), &iface );

    if (iface)
	codec = iface->createForMib(mib);

#endif // QT_NO_COMPONENT

    return codec;
}


#endif // QT_NO_TEXTCODEC
