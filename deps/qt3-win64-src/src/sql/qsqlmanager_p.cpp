/****************************************************************************
**
** Implementation of sql manager classes
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

#include "qsqlmanager_p.h"

#ifndef QT_NO_SQL

#include "qapplication.h"
#include "qwidget.h"
#include "qsqlcursor.h"
#include "qsqlform.h"
#include "qsqldriver.h"
#include "qstring.h"
#include "qmessagebox.h"
#include "qbitarray.h"

//#define QT_DEBUG_DATAMANAGER

class QSqlCursorManagerPrivate
{
public:
    QSqlCursorManagerPrivate()
	: cur( 0 ), autoDelete( FALSE )
    {}

    QString ftr;
    QStringList srt;
    QSqlCursor* cur;
    bool autoDelete;
};

/*!
  \class QSqlCursorManager qsqlmanager_p.h
  \brief The QSqlCursorManager class manages a database cursor.

  \module sql

  \internal

  This class provides common cursor management functionality.  This
  includes saving and applying sorts and filters, refreshing (i.e.,
  re-selecting) the cursor and searching for records within the
  cursor.

*/

/*!  \internal

  Constructs a cursor manager.

*/

QSqlCursorManager::QSqlCursorManager()
{
    d = new QSqlCursorManagerPrivate;
}


/*! \internal

  Destroys the object and frees any allocated resources.

*/

QSqlCursorManager::~QSqlCursorManager()
{
    if ( d->autoDelete )
	delete d->cur;
    delete d;
}

/*! \internal

  Sets the manager's sort to the index \a sort.  To apply the new
  sort, use refresh().

 */

void QSqlCursorManager::setSort( const QSqlIndex& sort )
{
    setSort( sort.toStringList() );
}

/*! \internal

  Sets the manager's sort to the stringlist \a sort.  To apply the
  new sort, use refresh().

 */

void QSqlCursorManager::setSort( const QStringList& sort )
{
    d->srt = sort;
}

/*! \internal

  Returns the current sort, or an empty stringlist if there is none.

*/

QStringList  QSqlCursorManager::sort() const
{
    return d->srt;
}

/*! \internal

  Sets the manager's filter to the string \a filter.  To apply the
  new filter, use refresh().

*/

void QSqlCursorManager::setFilter( const QString& filter )
{
    d->ftr = filter;
}

/*! \internal

  Returns the current filter, or an empty string if there is none.

*/

QString QSqlCursorManager::filter() const
{
    return d->ftr;
}

/*! \internal

  Sets auto-delete to \a enable.  If TRUE, the default cursor will
  be deleted when necessary.

  \sa autoDelete()
*/

void QSqlCursorManager::setAutoDelete( bool enable )
{
    d->autoDelete = enable;
}


/*! \internal

  Returns TRUE if auto-deletion is enabled, otherwise FALSE.

  \sa setAutoDelete()

*/

bool QSqlCursorManager::autoDelete() const
{
    return d->autoDelete;
}

/*! \internal

  Sets the default cursor used by the manager to \a cursor.  If \a
  autoDelete is TRUE (the default is FALSE), the manager takes
  ownership of the \a cursor pointer, which will be deleted when the
  manager is destroyed, or when setCursor() is called again. To
  activate the \a cursor use refresh().

  \sa cursor()

*/

void QSqlCursorManager::setCursor( QSqlCursor* cursor, bool autoDelete )
{
    if ( d->autoDelete )
	delete d->cur;
    d->cur = cursor;
    d->autoDelete = autoDelete;
}

/*! \internal

  Returns a pointer to the default cursor used for navigation, or 0
  if there is no default cursor.

  \sa setCursor()

*/

QSqlCursor* QSqlCursorManager::cursor() const
{
    return d->cur;
}


/*! \internal

  Refreshes the manager using the default cursor.  The manager's
  filter and sort are applied.  Returns TRUE on success, FALSE if an
  error occurred or there is no current cursor.

  \sa setFilter() setSort()

*/

bool QSqlCursorManager::refresh()
{
    QSqlCursor* cur = cursor();
    if ( !cur )
	return FALSE;
    QString currentFilter = d->ftr;
    QStringList currentSort = d->srt;
    QSqlIndex newSort = QSqlIndex::fromStringList( currentSort, cur );
    return cur->select( currentFilter, newSort );
}

/* \internal

   Returns TRUE if the \a buf field values that correspond to \a idx
   match the field values in \a cur that correspond to \a idx.
*/

static bool index_matches( const QSqlCursor* cur, const QSqlRecord* buf,
			   const QSqlIndex& idx )
{
    bool indexEquals = FALSE;
    for ( uint i = 0; i < idx.count(); ++i ) {
	const QString fn( idx.field(i)->name() );
	if ( cur->value( fn ) == buf->value( fn ) )
	    indexEquals = TRUE;
	else {
	    indexEquals = FALSE;
	    break;
	}
    }
    return indexEquals;
}

/*
  Return less than, equal to or greater than 0 if buf1 is less than,
  equal to or greater than buf2 according to fields described in idx.
  (### Currently only uses first field.)
*/

static int compare_recs( const QSqlRecord* buf1, const QSqlRecord* buf2,
			 const QSqlIndex& idx )
{
    int cmp = 0;

    int i = 0;
    const QString fn( idx.field(i)->name() );
    const QSqlField* f1 = buf1->field( fn );

    if ( f1 ) {
	switch ( f1->type() ) { // ### more types?
	case QVariant::String:
	case QVariant::CString:
	    cmp = f1->value().toString().simplifyWhiteSpace().compare(
		          buf2->value(fn).toString().simplifyWhiteSpace() );
	    break;
	default:
	    if ( f1->value().toDouble() < buf2->value( fn ).toDouble() )
		cmp = -1;
	    else if ( f1->value().toDouble() > buf2->value( fn ).toDouble() )
		cmp = 1;
	}
    }

    if ( idx.isDescending(i) )
	cmp = -cmp;
    return cmp;
}

#ifdef QT_DEBUG_DATAMANAGER
static void debug_datamanager_buffer( const QString& msg, QSqlRecord* cursor )
{
    qDebug("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    qDebug( "%s", msg.latin1() );
    for ( uint j = 0; j < cursor->count(); ++j ) {
	qDebug( "%s", (cursor->field(j)->name() + " type:"
		       + QString(cursor->field(j)->value().typeName())
		       + " value:" + cursor->field(j)->value().toString())
		       .latin1() );
    }
}
#endif


/*! \internal

  Relocates the default cursor to the record matching the cursor's
edit buffer.  Only the field names specified by \a idx are used to
determine an exact match of the cursor to the edit buffer. However,
other fields in the edit buffer are also used during the search,
therefore all fields in the edit buffer should be primed with desired
values for the record being sought.  This function is typically used
to relocate a cursor to the correct position after an insert or
update.  For example:

\code
    QSqlCursor* myCursor = myManager.cursor();
    ...
    QSqlRecord* buf = myCursor->primeUpdate();
    buf->setValue( "name", "Ola" );
    buf->setValue( "city", "Oslo" );
    ...
    myCursor->update();  // update current record
    myCursor->select();  // refresh the cursor
    myManager.findBuffer( myCursor->primaryIndex() ); // go to the updated record
\endcode

*/

//## possibly add sizeHint parameter
bool QSqlCursorManager::findBuffer( const QSqlIndex& idx, int atHint )
{
#ifdef QT_DEBUG_DATAMANAGER
    qDebug("QSqlCursorManager::findBuffer:");
#endif
    QSqlCursor* cur = cursor();
    if ( !cur )
	return FALSE;
    if ( !cur->isActive() )
	return FALSE;
    if ( !idx.count() ) {
	if ( cur->at() == QSql::BeforeFirst )
	    cur->next();
	return FALSE;
    }
    QSqlRecord* buf = cur->editBuffer();
    bool indexEquals = FALSE;
#ifdef QT_DEBUG_DATAMANAGER
    qDebug(" Checking hint...");
#endif
    /* check the hint */
    if ( cur->seek( atHint ) )
	indexEquals = index_matches( cur, buf, idx );

    if ( !indexEquals ) {
#ifdef QT_DEBUG_DATAMANAGER
	qDebug(" Checking current page...");
#endif
	/* check current page */
	int pageSize = 20;
	int startIdx = QMAX( atHint - pageSize, 0 );
	int endIdx = atHint + pageSize;
	for ( int j = startIdx; j <= endIdx; ++j ) {
	    if ( cur->seek( j ) ) {
		indexEquals = index_matches( cur, buf, idx );
		if ( indexEquals )
		    break;
	    }
	}
    }

    if ( !indexEquals && cur->driver()->hasFeature( QSqlDriver::QuerySize )
	 && cur->sort().count() ) {
#ifdef QT_DEBUG_DATAMANAGER
	qDebug(" Using binary search...");
#endif
	// binary search based on record buffer and current sort fields
	int lo = 0;
	int hi = cur->size();
	int mid;
	if ( compare_recs( buf, cur, cur->sort() ) >= 0 )
	    lo = cur->at();
	while ( lo != hi ) {
	    mid = lo + (hi - lo) / 2;
	    if ( !cur->seek( mid ) )
		break;
	    if ( index_matches( cur, buf, idx ) ) {
		indexEquals = TRUE;
		break;
	    }
	    int c = compare_recs( buf, cur, cur->sort() );
	    if ( c < 0 ) {
		hi = mid;
	    } else if ( c == 0 ) {
		// found it, but there may be duplicates
		int at = mid;
		do {
		    mid--;
		    if ( !cur->seek( mid ) )
			break;
		    if ( index_matches( cur, buf, idx ) ) {
			indexEquals = TRUE;
			break;
		    }
		} while ( compare_recs( buf, cur, cur->sort() ) == 0 );

		if ( !indexEquals ) {
		    mid = at;
		    do {
			mid++;
			if ( !cur->seek( mid ) )
			    break;
			if ( index_matches( cur, buf, idx ) ) {
			    indexEquals = TRUE;
			    break;
			}
		    } while ( compare_recs( buf, cur, cur->sort() ) == 0 );
		}
		break;
	    } else if ( c > 0 ) {
		lo = mid + 1;
	    }
	}
    }

    if ( !indexEquals ) {
#ifdef QT_DEBUG_DATAMANAGER
	qDebug(" Using brute search...");
#endif
#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor( Qt::waitCursor );
#endif
	/* give up, use brute force */
	int startIdx = 0;
	if ( cur->at() != startIdx ) {
	    cur->seek( startIdx );
	}
	for ( ;; ) {
	    indexEquals = FALSE;
	    indexEquals = index_matches( cur, buf, idx );
	    if ( indexEquals )
		break;
	    if ( !cur->next() )
		break;
	}
#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
#endif
    }
#ifdef QT_DEBUG_DATAMANAGER
	qDebug(" Done, result:" + QString::number( indexEquals ) );
#endif
    return indexEquals;
}

#ifndef QT_NO_SQL_FORM

class QSqlFormManagerPrivate
{
public:
    QSqlFormManagerPrivate() : frm(0), rcd(0) {}
    QSqlForm* frm;
    QSqlRecord* rcd;
};


/*! \internal

  Creates a form manager.

*/

QSqlFormManager::QSqlFormManager()
{
    d = new QSqlFormManagerPrivate();
}

/*! \internal

  Destroys the object and frees any allocated resources.

*/

QSqlFormManager::~QSqlFormManager()
{
    delete d;
}

/*!  \internal

  Clears the default form values.  If there is no default form,
  nothing happens,

*/

void QSqlFormManager::clearValues()
{
    if ( form() )
	form()->clearValues();
}

/*! \internal

  Sets the form used by the form manager to \a form.  If a record has
  already been assigned to the form manager, that record is also used by
  the \a form to display data.

  \sa form()

*/

void QSqlFormManager::setForm( QSqlForm* form )
{
    d->frm = form;
    if ( d->rcd && d->frm )
	d->frm->setRecord( d->rcd );
}


/*! \internal

  Returns the default form used by the form manager, or 0 if there is
  none.

  \sa setForm()

*/

QSqlForm* QSqlFormManager::form()
{
    return d->frm;
}


/*! \internal

  Sets the record used by the form manager to \a record.  If a form has
  already been assigned to the form manager, \a record is also used by
  the default form to display data.

  \sa record()

*/

void QSqlFormManager::setRecord( QSqlRecord* record )
{
    d->rcd = record;
    if ( d->frm ) {
	d->frm->setRecord( d->rcd );
    }
}


/*! \internal

  Returns the default record used by the form manager, or 0 if there is
  none.

  \sa setRecord()
*/

QSqlRecord* QSqlFormManager::record()
{
    return d->rcd;
}


/*! \internal

  Causes the default form to read its fields .  If there is no
  default form, nothing happens.

  \sa setForm()

*/

void QSqlFormManager::readFields()
{
    if ( d->frm ) {
	d->frm->readFields();
    }
}

/*! \internal

  Causes the default form to write its fields .  If there is no
  default form, nothing happens.

  \sa setForm()

*/

void QSqlFormManager::writeFields()
{
    if ( d->frm ) {
	d->frm->writeFields();
    }
}

#endif // QT_NO_SQL_FORM

class QDataManagerPrivate
{
public:
    QDataManagerPrivate()
	: mode( QSql::None ), autoEd( TRUE ), confEdits( 3 ),
	  confCancs( FALSE ) {}
    QSql::Op mode;
    bool autoEd;
    QBitArray confEdits;
    bool confCancs;

};

/*!
  \class QDataManager qsqlmanager_p.h
    \ingroup database

  \brief The QDataManager class is an internal class for implementing
  the data-aware widgets.

  \internal

  QDataManager is a strictly internal class that acts as a base class
  for other data-aware widgets.

*/


/*!  \internal

  Constructs an empty data handler.

*/

QDataManager::QDataManager()
{
    d = new QDataManagerPrivate();
}


/*! \internal

  Destroys the object and frees any allocated resources.

*/

QDataManager::~QDataManager()
{
    delete d;
}


/*!  \internal

  Virtual function which is called when an error has occurred The
  default implementation displays a warning message to the user with
  information about the error.

*/
void QDataManager::handleError( QWidget* parent, const QSqlError& e )
{
#ifndef QT_NO_MESSAGEBOX
    if (e.driverText().isEmpty() && e.databaseText().isEmpty()) {
	QMessageBox::warning ( parent, "Warning", "An error occurred while accessing the database");
    } else {
	QMessageBox::warning ( parent, "Warning", e.driverText() + "\n" + e.databaseText(),
			   0, 0 );
    }
#endif // QT_NO_MESSAGEBOX
}


/*! \internal

  Sets the internal mode to \a m.

*/

void QDataManager::setMode( QSql::Op m )
{
    d->mode = m;
}


/*! \internal

  Returns the current mode.

*/

QSql::Op QDataManager::mode() const
{
    return d->mode;
}


/*! \internal

  Sets the auto-edit mode to \a auto.

*/

void QDataManager::setAutoEdit( bool autoEdit )
{
    d->autoEd = autoEdit;
}



/*! \internal

  Returns TRUE if auto-edit mode is enabled; otherwise returns FALSE.

*/

bool QDataManager::autoEdit() const
{
    return d->autoEd;
}

/*! \internal

  If \a confirm is TRUE, all edit operations (inserts, updates and
  deletes) will be confirmed by the user.  If \a confirm is FALSE (the
  default), all edits are posted to the database immediately.

*/
void QDataManager::setConfirmEdits( bool confirm )
{
    d->confEdits.fill( confirm );
}

/*! \internal

  If \a confirm is TRUE, all inserts will be confirmed by the user.
  If \a confirm is FALSE (the default), all edits are posted to the
  database immediately.

*/

void QDataManager::setConfirmInsert( bool confirm )
{
    d->confEdits[ QSql::Insert ] = confirm;
}

/*! \internal

  If \a confirm is TRUE, all updates will be confirmed by the user.
  If \a confirm is FALSE (the default), all edits are posted to the
  database immediately.

*/

void QDataManager::setConfirmUpdate( bool confirm )
{
    d->confEdits[ QSql::Update ] = confirm;
}

/*! \internal

  If \a confirm is TRUE, all deletes will be confirmed by the user.
  If \a confirm is FALSE (the default), all edits are posted to the
  database immediately.

*/

void QDataManager::setConfirmDelete( bool confirm )
{
    d->confEdits[ QSql::Delete ] = confirm;
}

/*! \internal

  Returns TRUE if the table confirms all edit operations (inserts,
  updates and deletes), otherwise returns FALSE.
*/

bool QDataManager::confirmEdits() const
{
    return ( confirmInsert() && confirmUpdate() && confirmDelete() );
}

/*! \internal

  Returns TRUE if the table confirms inserts, otherwise returns
  FALSE.
*/

bool QDataManager::confirmInsert() const
{
    return d->confEdits[ QSql::Insert ];
}

/*! \internal

  Returns TRUE if the table confirms updates, otherwise returns
  FALSE.
*/

bool QDataManager::confirmUpdate() const
{
    return d->confEdits[ QSql::Update ];
}

/*! \internal

  Returns TRUE if the table confirms deletes, otherwise returns
  FALSE.
*/

bool QDataManager::confirmDelete() const
{
    return d->confEdits[ QSql::Delete ];
}

/*! \internal

  If \a confirm is TRUE, all cancels will be confirmed by the user
  through a message box.  If \a confirm is FALSE (the default), all
  cancels occur immediately.
*/

void QDataManager::setConfirmCancels( bool confirm )
{
    d->confCancs = confirm;
}

/*! \internal

  Returns TRUE if the table confirms cancels, otherwise returns FALSE.
*/

bool QDataManager::confirmCancels() const
{
    return d->confCancs;
}

/*!  \internal

  Virtual function which returns a confirmation for an edit of mode \a
  m.  Derived classes can reimplement this function and provide their
  own confirmation dialog.  The default implementation uses a message
  box which prompts the user to confirm the edit action.  The dialog
  is centered over \a parent.

*/

QSql::Confirm QDataManager::confirmEdit( QWidget* parent, QSql::Op m )
{
    int ans = 2;
    if ( m == QSql::Delete ) {
#ifndef QT_NO_MESSAGEBOX
	ans = QMessageBox::information( parent,
					qApp->translate( "QSql", "Delete" ),
					qApp->translate( "QSql", "Delete this record?" ),
					qApp->translate( "QSql", "Yes" ),
					qApp->translate( "QSql", "No" ),
					QString::null, 0, 1 );
#else
	ans = QSql::No;
#endif // QT_NO_MESSAGEBOX
    } else if ( m != QSql::None ) {
	QString caption;
	if ( m == QSql::Insert ) {
	    caption = qApp->translate( "QSql", "Insert" );
	} else { // QSql::Update
	    caption = qApp->translate( "QSql", "Update" );
	}
#ifndef QT_NO_MESSAGEBOX
	ans = QMessageBox::information( parent, caption,
					qApp->translate( "QSql", "Save edits?" ),
					qApp->translate( "QSql", "Yes" ),
					qApp->translate( "QSql", "No" ),
					qApp->translate( "QSql", "Cancel" ),
					0, 2 );
#else
	ans = QSql::No;
#endif // QT_NO_MESSAGEBOX
    }

    switch ( ans ) {
    case 0:
	return QSql::Yes;
    case 1:
	return QSql::No;
    default:
	return QSql::Cancel;
    }
}

/*!  \internal

  Virtual function which returns a confirmation for cancelling an edit
  mode \a m.  Derived classes can reimplement this function and
  provide their own confirmation dialog.  The default implementation
  uses a message box which prompts the user to confirm the edit
  action. The dialog is centered over \a parent.


*/

QSql::Confirm QDataManager::confirmCancel( QWidget* parent, QSql::Op )
{
#ifndef QT_NO_MESSAGEBOX
    switch ( QMessageBox::information( parent,
				       qApp->translate( "QSql", "Confirm" ),
				       qApp->translate( "QSql", "Cancel your edits?" ),
				       qApp->translate( "QSql", "Yes" ),
				       qApp->translate( "QSql", "No" ),
				       QString::null, 0, 1 ) ) {
    case 0:
	return QSql::Yes;
    case 1:
	return QSql::No;
    default:
	return QSql::Cancel;
    }
#else
    return QSql::Yes;
#endif // QT_NO_MESSAGEBOX
}

#endif
