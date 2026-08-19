/****************************************************************************
** $Id: qobjectcleanuphandler.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QObjectCleanupHandler class
**
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
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

#include "qobjectcleanuphandler.h"
#include "qobjectlist.h"

/*!
    \class QObjectCleanupHandler qobjectcleanuphandler.h
    \brief The QObjectCleanupHandler class watches the lifetime of multiple QObjects.

    \ingroup objectmodel

    A QObjectCleanupHandler is useful whenever you need to know when a
    number of \l{QObject}s that are owned by someone else have been
    deleted. This is important, for example, when referencing memory
    in an application that has been allocated in a shared library.

    Example:

    \code
    class FactoryComponent : public FactoryInterface, public QLibraryInterface
    {
    public:
	...

	QObject *createObject();

	bool init();
	void cleanup();
	bool canUnload() const;

    private:
	QObjectCleanupHandler objects;
    };

    // allocate a new object, and add it to the cleanup handler
    QObject *FactoryComponent::createObject()
    {
	return objects.add( new QObject() );
    }

    // QLibraryInterface implementation
    bool FactoryComponent::init()
    {
	return TRUE;
    }

    void FactoryComponent::cleanup()
    {
    }

    // it is only safe to unload the library when all QObject's have been destroyed
    bool FactoryComponent::canUnload() const
    {
	return objects.isEmpty();
    }
    \endcode
*/

/*!
    Constructs an empty QObjectCleanupHandler.
*/
QObjectCleanupHandler::QObjectCleanupHandler()
: QObject(), cleanupObjects( 0 )
{
}

/*!
    Destroys the cleanup handler. All objects in this cleanup handler
    will be deleted.
*/
QObjectCleanupHandler::~QObjectCleanupHandler()
{
    clear();
}

/*!
    Adds \a object to this cleanup handler and returns the pointer to
    the object.
*/
QObject* QObjectCleanupHandler::add( QObject* object )
{
    if ( !object )
	return 0;

    if ( !cleanupObjects ) {
	cleanupObjects = new QObjectList;
 	cleanupObjects->setAutoDelete( TRUE );
    }
    connect( object, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)) );
    cleanupObjects->insert( 0, object );
    return object;
}

/*!
    Removes the \a object from this cleanup handler. The object will
    not be destroyed.
*/
void QObjectCleanupHandler::remove( QObject *object )
{
    if ( !cleanupObjects )
	return;
    if ( cleanupObjects->findRef( object ) >= 0 ) {
	(void) cleanupObjects->take();
	disconnect( object, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)) );
    }
}

/*!
    Returns TRUE if this cleanup handler is empty or if all objects in
    this cleanup handler have been destroyed; otherwise return FALSE.
*/
bool QObjectCleanupHandler::isEmpty() const
{
    return cleanupObjects ? cleanupObjects->isEmpty() : TRUE;
}

/*!
    Deletes all objects in this cleanup handler. The cleanup handler
    becomes empty.
*/
void QObjectCleanupHandler::clear()
{
    delete cleanupObjects;
    cleanupObjects = 0;
}

void QObjectCleanupHandler::objectDestroyed( QObject*object )
{
    if ( cleanupObjects )
	cleanupObjects->setAutoDelete( FALSE );

    remove( object );

    if ( cleanupObjects )
	cleanupObjects->setAutoDelete( TRUE );
}
