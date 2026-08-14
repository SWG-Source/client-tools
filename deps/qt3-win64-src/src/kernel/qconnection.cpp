/****************************************************************************
** $Id: qconnection.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QConnection class
**
** Created : 930417
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
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

#include "qconnection.h"

/*! \class QConnection qconnection.h
  \brief The QConnection class is an internal class, used in the signal/slot mechanism.

  \internal

  Do not use this class directly in application programs.

  QObject has a list of QConnection for each signal that is connected to the
  outside world.
*/

QConnection::QConnection( const QObject *object, int member,
			  const char *memberName, int memberType )
{
    obj = (QObject *)object;
    mbr = member;
    mbr_name = memberName;
    mbr_type = memberType;
    nargs = 0;
    if ( strstr(memberName,"()") == 0 ) {
	const char *p = memberName;
	nargs++;
	while ( *p ) {
	    if ( *p++ == ',' )
		nargs++;
	}
    }
}

/*!
 \fn QConnection::~QConnection()
*/

/*!
  \fn bool QConnection::isConnected() const
*/

/*!
  \fn QObject *QConnection::object() const
*/

/*!
  \fn int QConnection::member() const
*/

/*!
  \fn const char *QConnection::memberName() const
*/

/*!
  \fn int QConnection::numArgs() const
*/
