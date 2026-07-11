/****************************************************************************
**
** Implementation of QSqlError class
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

#include "qsqlerror.h"
#include <qmessagebox.h>

#ifndef QT_NO_SQL

/*!
    \class QSqlError qsqlerror.h
    \brief The QSqlError class provides SQL database error information.

    \ingroup database
    \module sql

    This class is used to report database-specific errors. An error
    description and (if appropriate) a database-specific error number
    can be obtained using this class.
*/

/*!
    \enum QSqlError::Type

    This enum type describes the type of SQL error that occurred.

    \value None  no error occurred
    \value Connection  connection error
    \value Statement  SQL statement syntax error
    \value Transaction  transaction failed error
    \value Unknown  unknown error
*/

/*!
    Constructs an error containing the driver error text \a
    driverText, the database-specific error text \a databaseText, the
    type \a type and the optional error number \a number.
*/

QSqlError::QSqlError(  const QString& driverText,
    		const QString& databaseText,
		int type,
		int number )
: driverError(driverText),
  databaseError(databaseText),
  errorType(type),
  errorNumber(number)
{
}

/*!
    Creates a copy of \a other.
*/

QSqlError::QSqlError( const QSqlError& other )
: driverError(other.driverError),
  databaseError(other.databaseError),
  errorType(other.errorType),
  errorNumber(other.errorNumber)
{
}

/*!
    Sets the error equal to \a other.
*/

QSqlError& QSqlError::operator=( const QSqlError& other )
{
    driverError = other.driverError;
    databaseError = other.databaseError;
    errorType = other.errorType;
    errorNumber = other.errorNumber;
    return *this;
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlError::~QSqlError()
{
}

/*!
    Returns the text of the error as reported by the driver. This may
    contain database-specific descriptions.
*/
QString QSqlError::driverText() const
{
    return driverError;
}

/*!
    Sets the driver error text to the value of \a driverText.
*/

void QSqlError::setDriverText( const QString& driverText )
{
    driverError = driverText;
}

/*!
    Returns the text of the error as reported by the database. This
    may contain database-specific descriptions.
*/

QString QSqlError::databaseText() const
{
    return databaseError;
}

/*!
    Sets the database error text to the value of \a databaseText.
*/

void QSqlError::setDatabaseText( const QString& databaseText )
{
    databaseError = databaseText;
}

/*!
    Returns the error type, or -1 if the type cannot be determined.

    \sa QSqlError::Type.
*/

int QSqlError::type() const
{
    return errorType;
}

/*!
    Sets the error type to the value of \a type.
*/

void QSqlError::setType( int type )
{
    errorType = type;
}

/*!
    Returns the database-specific error number, or -1 if it cannot be
    determined.
*/

int QSqlError::number() const
{
    return errorNumber;
}

/*!
    Sets the database-specific error number to \a number.
*/

void QSqlError::setNumber( int number )
{
    errorNumber = number;
}

/*!
    This is a convenience function that returns databaseText() and
    driverText() concatenated into a single string.

    \sa showMessage(), driverText(), databaseText()
*/

QString QSqlError::text() const
{
    if ( databaseError.endsWith("\n") )
	return databaseError + driverError;
    else
	return databaseError + " " + driverError;
}

/*!
    \obsolete
    
    This is a convenience function that pops up a QMessageBox
    containing the message returned by text(). An additional string
    can be passed in via the \a msg parameter, which will be
    concatenated with the text() message.

    \sa text(), driverText(), databaseText()
*/
void QSqlError::showMessage( const QString& msg ) const
{
#ifndef QT_NO_MESSAGEBOX
    QMessageBox::warning( NULL, "SQL Error", msg + text(), 
			  QMessageBox::Ok, QMessageBox::NoButton ); 
#endif // QT_NO_MESSAGEBOX
}
#endif // QT_NO_SQL
