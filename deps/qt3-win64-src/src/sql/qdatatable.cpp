/****************************************************************************
**
** Implementation of QDataTable class
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

#include "qdatatable.h"

#ifndef QT_NO_SQL_VIEW_WIDGETS

#include "qsqldriver.h"
#include "qsqleditorfactory.h"
#include "qsqlpropertymap.h"
#include "qapplication.h"
#include "qlayout.h"
#include "qpainter.h"
#include "qpopupmenu.h"
#include "qvaluelist.h"
#include "qsqlmanager_p.h"
#include "qdatetime.h"
#include "qcursor.h"
#include "qtimer.h"

//#define QT_DEBUG_DATATABLE

class QDataTablePrivate
{
public:
    QDataTablePrivate()
	: nullTxtChanged( FALSE ),
	  haveAllRows( FALSE ),
	  continuousEdit( FALSE ),
	  editorFactory( 0 ),
	  propertyMap( 0 ),
	  editRow( -1 ),
	  editCol( -1 ),
	  insertRowLast( -1 ),
	  insertPreRows( -1 ),
	  editBuffer( 0 ),
	  cancelMode( FALSE ),
	  cancelInsert( FALSE ),
	  cancelUpdate( FALSE )
    {}
    ~QDataTablePrivate() { if ( propertyMap ) delete propertyMap; }

    QString nullTxt;
    bool nullTxtChanged;
    typedef QValueList< uint > ColIndex;
    ColIndex colIndex;
    bool haveAllRows;
    bool continuousEdit;
    QSqlEditorFactory* editorFactory;
    QSqlPropertyMap* propertyMap;
    QString trueTxt;
    Qt::DateFormat datefmt;
    QString falseTxt;
    int editRow;
    int editCol;
    int insertRowLast;
    QString insertHeaderLabelLast;
    int insertPreRows;
    QSqlRecord* editBuffer;
    bool cancelMode;
    bool cancelInsert;
    bool cancelUpdate;
    int lastAt;
    QString ftr;
    QStringList srt;
    QStringList fld;
    QStringList fldLabel;
    QValueList<int> fldWidth;
    QValueList<QIconSet> fldIcon;
    QValueList<bool> fldHidden;
    QSqlCursorManager cur;
    QDataManager dat;
};

#ifdef QT_DEBUG_DATATABLE
void qt_debug_buffer( const QString& msg, QSqlRecord* cursor )
{
    qDebug("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    qDebug(msg);
    for ( uint j = 0; j < cursor->count(); ++j ) {
	qDebug(cursor->field(j)->name() + " type:" + QString(cursor->field(j)->value().typeName()) + " value:" + cursor->field(j)->value().toString() );
    }
}
#endif

/*!
    \enum QDataTable::Refresh

    This enum describes the refresh options.

    \value RefreshData  refresh the data, i.e. read it from the database
    \value RefreshColumns  refresh the list of fields, e.g. the column headings
    \value RefreshAll  refresh both the data and the list of fields
*/


/*!
    \class QDataTable qdatatable.h
    \brief The QDataTable class provides a flexible SQL table widget that supports browsing and editing.

    \ingroup database
    \mainclass
    \module sql

    QDataTable supports various functions for presenting and editing
    SQL data from a \l QSqlCursor in a table.

    If you want a to present your data in a form use QDataBrowser, or
    for read-only forms, QDataView.

    When displaying data, QDataTable only retrieves data for visible
    rows. If the driver supports the 'query size' property the
    QDataTable will have the correct number of rows and the vertical
    scrollbar will accurately reflect the number of rows displayed in
    proportion to the number of rows in the dataset. If the driver
    does not support the 'query size' property, rows are dynamically
    fetched from the database on an as-needed basis with the scrollbar
    becoming more accurate as the user scrolls down through the
    records. This allows extremely large queries to be displayed as
    quickly as possible, with minimum memory usage.

    QDataTable inherits QTable's API and extends it with functions to
    sort and filter the data and sort columns. See setSqlCursor(),
    setFilter(), setSort(), setSorting(), sortColumn() and refresh().

    When displaying editable cursors, cell editing will be enabled.
    (For more information on editable cursors, see \l QSqlCursor).
    QDataTable can be used to modify existing data and to add new
    records. When a user makes changes to a field in the table, the
    cursor's edit buffer is used. The table will not send changes in
    the edit buffer to the database until the user moves to a
    different record in the grid or presses Enter. Cell editing is
    initiated by pressing F2 (or right clicking and then clicking the
    appropriate popup menu item) and canceled by pressing Esc. If
    there is a problem updating or adding data, errors are handled
    automatically (see handleError() to change this behavior). Note
    that if autoEdit() is FALSE navigating to another record will
    cancel the insert or update.

    The user can be asked to confirm all edits with setConfirmEdits().
    For more precise control use setConfirmInsert(),
    setConfirmUpdate(), setConfirmDelete() and setConfirmCancels().
    Use setAutoEdit() to control the behaviour of the table when the
    user edits a record and then navigates. (Note that setAutoDelete()
    is unrelated; it is used to set whether the QSqlCursor is deleted
    when the table is deleted.)

    Since the data table can perform edits, it must be able to
    uniquely identify every record so that edits are correctly
    applied. Because of this the underlying cursor must have a valid
    primary index to ensure that a unique record is inserted, updated
    or deleted within the database otherwise the database may be
    changed to an inconsistent state.

    QDataTable creates editors using the default \l QSqlEditorFactory.
    Different editor factories can be used by calling
    installEditorFactory(). A property map is used to map between the
    cell's value and the editor. You can use your own property map
    with installPropertyMap().

    The contents of a cell is available as a QString with text() or as
    a QVariant with value(). The current record is returned by
    currentRecord(). Use the find() function to search for a string in
    the table.

    Editing actions can be applied programatically. For example, the
    insertCurrent() function reads the fields from the current record
    into the cursor and performs the insert. The updateCurrent() and
    deleteCurrent() functions perform similarly to update and delete
    the current record respectively.

    Columns in the table can be created automatically based on the
    cursor (see setSqlCursor()). Columns can be manipulated manually
    using addColumn(), removeColumn() and setColumn().

    The table automatically copies many of the properties of the
    cursor to format the display of data within cells (alignment,
    visibility, etc.). The cursor can be changed with setSqlCursor().
    The filter (see setFilter()) and sort defined within the table are
    used instead of the filter and sort set on the cursor. For sorting
    options see setSort(), sortColumn(), sortAscending() and
    sortDescending(). Note that sorting operations will not behave as
    expected if you are using a QSqlSelectCursor because it uses
    user-defined SQL queries to obtain data.

    The text used to represent NULL, TRUE and FALSE values can be
    changed with setNullText(), setTrueText() and setFalseText()
    respectively. You can change the appearance of cells by
    reimplementing paintField().

    Whenever a new row is selected in the table the currentChanged()
    signal is emitted. The primeInsert() signal is emitted when an
    insert is initiated. The primeUpdate() and primeDelete() signals
    are emitted when update and deletion are initiated respectively.
    Just before the database is updated a signal is emitted;
    beforeInsert(), beforeUpdate() or beforeDelete() as appropriate.

*/

/*!
    Constructs a data table which is a child of \a parent, called
    name \a name.
*/

QDataTable::QDataTable ( QWidget * parent, const char * name )
    : QTable( parent, name )
{
    init();
}

/*!
    Constructs a data table which is a child of \a parent, called name
    \a name using the cursor \a cursor.

    If \a autoPopulate is TRUE (the default is FALSE), columns are
    automatically created based upon the fields in the \a cursor
    record. Note that \a autoPopulate only governs the creation of
    columns; to load the cursor's data into the table use refresh().

    If the \a cursor is read-only, the table also becomes read-only.
    In addition, the table adopts the cursor's driver's definition for
    representing NULL values as strings.
*/

QDataTable::QDataTable ( QSqlCursor* cursor, bool autoPopulate, QWidget * parent, const char * name )
    : QTable( parent, name )
{
    init();
    setSqlCursor( cursor, autoPopulate );
}

/*! \internal
*/


void QDataTable::init()
{
    d = new QDataTablePrivate();
    setAutoEdit( TRUE );
    setSelectionMode( SingleRow );
    setFocusStyle( FollowStyle );
    d->trueTxt = tr( "True" );
    d->falseTxt = tr( "False" );
    d->datefmt = Qt::LocalDate;
    reset();
    connect( this, SIGNAL( selectionChanged() ),
	     SLOT( updateCurrentSelection()));
}

/*!
    Destroys the object and frees any allocated resources.
*/

QDataTable::~QDataTable()
{
    delete d;
}


/*!
    Adds the next column to be displayed using the field \a fieldName,
    column label \a label, width \a width and iconset \a iconset.

    If \a label is specified, it is used as the column's header label,
    otherwise the field's display label is used when setSqlCursor() is
    called. The \a iconset is used to set the icon used by the column
    header; by default there is no icon.

    \sa setSqlCursor() refresh()
*/

void QDataTable::addColumn( const QString& fieldName,
			    const QString& label,
			    int width,
			    const QIconSet& iconset )
{
    d->fld += fieldName;
    d->fldLabel += label;
    d->fldIcon += iconset;
    d->fldWidth += width;
    d->fldHidden += FALSE;
}

/*!
    Sets the \a col column to display using the field \a fieldName,
    column label \a label, width \a width and iconset \a iconset.

    If \a label is specified, it is used as the column's header label,
    otherwise the field's display label is used when setSqlCursor() is
    called. The \a iconset is used to set the icon used by the column
    header; by default there is no icon.

    \sa setSqlCursor() refresh()
*/

void QDataTable::setColumn( uint col, const QString& fieldName,
			    const QString& label,
			    int width,
			    const QIconSet& iconset )
{
    d->fld[col]= fieldName;
    d->fldLabel[col] = label;
    d->fldIcon[col] = iconset;
    d->fldWidth[col] = width;
    d->fldHidden[col] = FALSE;
}

/*!
    Removes column \a col from the list of columns to be displayed. If
    \a col does not exist, nothing happens.

    \sa QSqlField
*/

void QDataTable::removeColumn( uint col )
{
    if ( d->fld.at( col ) != d->fld.end() ) {
	d->fld.remove( d->fld.at( col ) );
	d->fldLabel.remove( d->fldLabel.at( col ) );
	d->fldIcon.remove( d->fldIcon.at( col ) );
	d->fldWidth.remove( d->fldWidth.at( col ) );
	d->fldHidden.remove( d->fldHidden.at( col ) );
    }
}

/*!
    Sets the column \a col to the width \a w. Note that unlike QTable
    the QDataTable is not immediately redrawn, you must call
    refresh(QDataTable::RefreshColumns)
    yourself.

    \sa refresh()
*/
void QDataTable::setColumnWidth( int col, int w )
{
    if ( d->fldWidth.at( col ) != d->fldWidth.end() ) {
	d->fldWidth[col] = w;
    }
}

/*!
    Resizes column \a col so that the column width is wide enough to
    display the widest item the column contains (including the column
    label). If the table's QSqlCursor is not currently active, the
    cursor will be refreshed before the column width is calculated. Be
    aware that this function may be slow on tables that contain large
    result sets.
*/
void QDataTable::adjustColumn( int col )
{
    QSqlCursor * cur = sqlCursor();
    if ( !cur || cur->count() <= (uint)col )
	return;
    if ( !cur->isActive() ) {
	d->cur.refresh();
    }
    int oldRow = currentRow();
    int w = fontMetrics().width( horizontalHeader()->label( col ) + "W" );
    cur->seek( QSql::BeforeFirst );
    while ( cur->next() ) {
	w = QMAX( w, fontMetrics().width( fieldToString( cur->field( indexOf( col ) ) ) ) + 10 );
    }
    setColumnWidth( col, w );
    cur->seek( oldRow );
    refresh( RefreshColumns );
}

/*! \reimp
*/
void QDataTable::setColumnStretchable( int col, bool s )
{
    if ( numCols() == 0 ) {
	refresh( RefreshColumns );
    }
    if ( numCols() > col ) {
	QTable::setColumnStretchable( col, s );
    }
}

QString QDataTable::filter() const
{
    return d->cur.filter();
}

/*!
    \property QDataTable::filter
    \brief the data filter for the data table

    The filter applies to the data shown in the table. To view data
    with a new filter, use refresh(). A filter string is an SQL WHERE
    clause without the WHERE keyword.

    There is no default filter.

    \sa sort()

*/

void QDataTable::setFilter( const QString& filter )
{
    d->cur.setFilter( filter );
}


/*!
    \property QDataTable::sort
    \brief the data table's sort

    The table's sort affects the order in which data records are
    displayed in the table. To apply a sort, use refresh().

    When examining the sort property, a string list is returned with
    each item having the form 'fieldname order' (e.g., 'id ASC',
    'surname DESC').

    There is no default sort.

    Note that if you want to iterate over the sort list, you should
    iterate over a copy, e.g.
    \code
    QStringList list = myDataTable.sort();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

    \sa filter() refresh()
*/

void QDataTable::setSort( const QStringList& sort )
{
    d->cur.setSort( sort );
}

/*!
    \overload

    Sets the sort to be applied to the displayed data to \a sort. If
    there is no current cursor, nothing happens. A QSqlIndex contains
    field names and their ordering (ASC or DESC); these are used to
    compose the ORDER BY clause.

    \sa sort()
*/

void QDataTable::setSort( const QSqlIndex& sort )
{
    d->cur.setSort( sort );
}

QStringList QDataTable::sort() const
{
    return d->cur.sort();
}

/*!
    Returns the cursor used by the data table.
*/

QSqlCursor* QDataTable::sqlCursor() const
{
    return d->cur.cursor();
}

void QDataTable::setConfirmEdits( bool confirm )
{
    d->dat.setConfirmEdits( confirm );
}

void QDataTable::setConfirmInsert( bool confirm )
{
    d->dat.setConfirmInsert( confirm );
}

void QDataTable::setConfirmUpdate( bool confirm )
{
    d->dat.setConfirmUpdate( confirm );
}

void QDataTable::setConfirmDelete( bool confirm )
{
    d->dat.setConfirmDelete( confirm );
}

/*!
    \property QDataTable::confirmEdits
    \brief whether the data table confirms edit operations

    If the confirmEdits property is TRUE, the data table confirms all
    edit operations (inserts, updates and deletes). Finer control of
    edit confirmation can be achieved using \l confirmCancels, \l
    confirmInsert, \l confirmUpdate and \l confirmDelete.

    \sa confirmCancels() confirmInsert() confirmUpdate() confirmDelete()
*/

bool QDataTable::confirmEdits() const
{
    return ( d->dat.confirmEdits() );
}

/*!
    \property QDataTable::confirmInsert
    \brief whether the data table confirms insert operations

    If the confirmInsert property is TRUE, all insertions must be
    confirmed by the user through a message box (this behaviour can be
    changed by overriding the confirmEdit() function), otherwise all
    insert operations occur immediately.

    \sa confirmCancels() confirmEdits() confirmUpdate() confirmDelete()
*/

bool QDataTable::confirmInsert() const
{
    return ( d->dat.confirmInsert() );
}

/*!
    \property QDataTable::confirmUpdate
    \brief whether the data table confirms update operations

    If the confirmUpdate property is TRUE, all updates must be
    confirmed by the user through a message box (this behaviour can be
    changed by overriding the confirmEdit() function), otherwise all
    update operations occur immediately.

    \sa confirmCancels() confirmEdits() confirmInsert() confirmDelete()
*/

bool QDataTable::confirmUpdate() const
{
    return ( d->dat.confirmUpdate() );
}

/*!
    \property QDataTable::confirmDelete
    \brief whether the data table confirms delete operations

    If the confirmDelete property is TRUE, all deletions must be
    confirmed by the user through a message box (this behaviour can be
    changed by overriding the confirmEdit() function), otherwise all
    delete operations occur immediately.

    \sa confirmCancels() confirmEdits() confirmUpdate() confirmInsert()
*/

bool QDataTable::confirmDelete() const
{
    return ( d->dat.confirmDelete() );
}

/*!
    \property QDataTable::confirmCancels
    \brief whether the data table confirms cancel operations

    If the confirmCancel property is TRUE, all cancels must be
    confirmed by the user through a message box (this behavior can be
    changed by overriding the confirmCancel() function), otherwise all
    cancels occur immediately. The default is FALSE.

    \sa confirmEdits() confirmCancel()
*/

void QDataTable::setConfirmCancels( bool confirm )
{
    d->dat.setConfirmCancels( confirm );
}

bool QDataTable::confirmCancels() const
{
    return d->dat.confirmCancels();
}

/*!
    \reimp

    For an editable table, creates an editor suitable for the field in
    column \a col. The editor is created using the default editor
    factory, unless a different editor factory was installed with
    installEditorFactory(). The editor is primed with the value of the
    field in \a col using a property map. The property map used is the
    default property map, unless a new property map was installed with
    installPropertMap(). If \a initFromCell is TRUE then the editor is
    primed with the value in the QDataTable cell.
*/

QWidget * QDataTable::createEditor( int , int col, bool initFromCell ) const
{
    if ( d->dat.mode() == QSql::None )
	return 0;

    QSqlEditorFactory * f = (d->editorFactory == 0) ?
		     QSqlEditorFactory::defaultFactory() : d->editorFactory;

    QSqlPropertyMap * m = (d->propertyMap == 0) ?
			  QSqlPropertyMap::defaultMap() : d->propertyMap;

    QWidget * w = 0;
    if( initFromCell && d->editBuffer ){
	w = f->createEditor( viewport(), d->editBuffer->field( indexOf( col ) ) );
	if ( w )
	    m->setProperty( w, d->editBuffer->value( indexOf( col ) ) );
    }
    return w;
}

/*! \reimp */
bool QDataTable::eventFilter( QObject *o, QEvent *e )
{
    if ( d->cancelMode )
	return TRUE;

    int r = currentRow();
    int c = currentColumn();

    if ( d->dat.mode() != QSql::None ) {
	r = d->editRow;
	c = d->editCol;
    }

    d->cancelInsert = FALSE;
    d->cancelUpdate = FALSE;
    switch ( e->type() ) {
    case QEvent::KeyPress: {
	int conf = QSql::Yes;
	QKeyEvent *ke = (QKeyEvent*)e;
	if ( ( ke->key() == Key_Tab || ke->key() == Qt::Key_BackTab )
	    && ke->state() & Qt::ControlButton )
	    return FALSE;

	if ( ke->key() == Key_Escape && d->dat.mode() == QSql::Insert ){
	    if ( confirmCancels() && !d->cancelMode ) {
		d->cancelMode = TRUE;
		conf = confirmCancel( QSql::Insert );
		d->cancelMode = FALSE;
	    }
	    if ( conf == QSql::Yes ) {
		d->cancelInsert = TRUE;
	    } else {
		QWidget *editorWidget = cellWidget( r, c );
		if ( editorWidget ) {
		    editorWidget->setActiveWindow();
		    editorWidget->setFocus();
		}
		return TRUE;
	    }
	}
	if ( ke->key() == Key_Escape && d->dat.mode() == QSql::Update ) {
	    if ( confirmCancels() && !d->cancelMode ) {
		d->cancelMode = TRUE;
		conf = confirmCancel( QSql::Update );
		d->cancelMode = FALSE;
	    }
	    if ( conf == QSql::Yes ){
		d->cancelUpdate = TRUE;
	    } else {
		QWidget *editorWidget = cellWidget( r, c );
		if ( editorWidget ) {
		    editorWidget->setActiveWindow();
		    editorWidget->setFocus();
		}
		return TRUE;
	    }
	}
	if ( ke->key() == Key_Insert && d->dat.mode() == QSql::None ) {
	    beginInsert();
	    return TRUE;
	}
	if ( ke->key() == Key_Delete && d->dat.mode() == QSql::None ) {
	    deleteCurrent();
	    return TRUE;
	}
	if ( d->dat.mode() != QSql::None ) {
	    if ( (ke->key() == Key_Tab) && (c < numCols() - 1) && (!isColumnReadOnly( c+1 ) || d->dat.mode() == QSql::Insert) )
		d->continuousEdit = TRUE;
	    else if ( (ke->key() == Key_BackTab) && (c > 0) && (!isColumnReadOnly( c-1 ) || d->dat.mode() == QSql::Insert) )
		d->continuousEdit = TRUE;
	    else
		d->continuousEdit = FALSE;
	}
	QSqlCursor * sql = sqlCursor();
	if ( sql && sql->driver() &&
	     !sql->driver()->hasFeature( QSqlDriver::QuerySize ) &&
	     ke->key() == Key_End && d->dat.mode() == QSql::None ) {
#ifndef QT_NO_CURSOR
	    QApplication::setOverrideCursor( Qt::WaitCursor );
#endif
	    int i = sql->at();
	    if ( i < 0 ) {
		i = 0;
		sql->seek(0);
	    }
	    while ( sql->next() )
		i++;
	    setNumRows( i+1 );
	    setCurrentCell( i+1, currentColumn() );
#ifndef QT_NO_CURSOR
	    QApplication::restoreOverrideCursor();
#endif
	    return TRUE;
	}
	break;
    }
    case QEvent::FocusOut: {
	QWidget *editorWidget = cellWidget( r, c );
	repaintCell( currentRow(), currentColumn() );
	if ( !d->cancelMode && editorWidget && o == editorWidget &&
	     ( d->dat.mode() == QSql::Insert) && !d->continuousEdit) {
	    setCurrentCell( r, c );
	    d->cancelInsert = TRUE;
	}
	d->continuousEdit = FALSE;
	break;
    }
    case QEvent::FocusIn:
	repaintCell( currentRow(), currentColumn() );
	break;
    default:
	break;
    }
    return QTable::eventFilter( o, e );
}

/*! \reimp */
void QDataTable::resizeEvent ( QResizeEvent * e )
{
    if ( sqlCursor() &&
	 sqlCursor()->driver() &&
	 !sqlCursor()->driver()->hasFeature( QSqlDriver::QuerySize ) )
	loadNextPage();
    QTable::resizeEvent( e );
}

/*! \reimp */
void QDataTable::contentsContextMenuEvent( QContextMenuEvent* e )
{
    QTable::contentsContextMenuEvent( e );
    if ( isEditing() && d->dat.mode() != QSql::None )
	endEdit( d->editRow, d->editCol, autoEdit(), FALSE );
    if ( !sqlCursor() )
	return;
    if ( d->dat.mode() == QSql::None ) {
	if ( isReadOnly() )
	    return;
	enum {
	    IdInsert,
	    IdUpdate,
	    IdDelete
	};
	QGuardedPtr<QPopupMenu> popup = new QPopupMenu( this, "qt_datatable_menu" );
	int id[ 3 ];
	id[ IdInsert ] = popup->insertItem( tr( "Insert" ) );
	id[ IdUpdate ] = popup->insertItem( tr( "Update" ) );
	id[ IdDelete ] = popup->insertItem( tr( "Delete" ) );
	bool enableInsert = sqlCursor()->canInsert();
	popup->setItemEnabled( id[ IdInsert ], enableInsert );
	bool enableUpdate = currentRow() > -1 && sqlCursor()->canUpdate() && !isColumnReadOnly( currentColumn() );
	popup->setItemEnabled( id[ IdUpdate ], enableUpdate );
	bool enableDelete = currentRow() > -1 && sqlCursor()->canDelete();
	popup->setItemEnabled( id[ IdDelete ], enableDelete );
	int r = popup->exec( e->globalPos() );
	delete (QPopupMenu*) popup;
	if ( r == id[ IdInsert ] )
	    beginInsert();
	else if ( r == id[ IdUpdate ] ) {
	    if ( beginEdit( currentRow(), currentColumn(), FALSE ) )
		setEditMode( Editing, currentRow(), currentColumn() );
	    else
		endUpdate();
	}
	else if ( r == id[ IdDelete ] )
	    deleteCurrent();
	e->accept();
    }
}

/*! \reimp */
void QDataTable::contentsMousePressEvent( QMouseEvent* e )
{
    QTable::contentsMousePressEvent( e );
}

/*! \reimp */
QWidget* QDataTable::beginEdit ( int row, int col, bool replace )
{
    d->editRow = -1;
    d->editCol = -1;
    if ( !sqlCursor() )
	return 0;
    if ( d->dat.mode() == QSql::Insert && !sqlCursor()->canInsert() )
	return 0;
    if ( d->dat.mode() == QSql::Update && !sqlCursor()->canUpdate() )
	return 0;
    d->editRow = row;
    d->editCol = col;
    if ( d->continuousEdit ) {
	// see comment in beginInsert()
	bool fakeReadOnly = isColumnReadOnly( col );
	setColumnReadOnly( col, FALSE );
	QWidget* w = QTable::beginEdit( row, col, replace );
	setColumnReadOnly( col, fakeReadOnly );
	return w;
    }
    if ( d->dat.mode() == QSql::None && sqlCursor()->canUpdate() && sqlCursor()->primaryIndex().count() > 0 )
	return beginUpdate( row, col, replace );
    return 0;
}

/*! \reimp */
void QDataTable::endEdit( int row, int col, bool, bool )
{
    bool accept = autoEdit() && !d->cancelInsert && !d->cancelUpdate;

    QWidget *editor = cellWidget( row, col );
    if ( !editor )
	return;
    if ( d->cancelMode )
	return;
    if ( d->dat.mode() != QSql::None && d->editBuffer ) {
	QSqlPropertyMap * m = (d->propertyMap == 0) ?
			      QSqlPropertyMap::defaultMap() : d->propertyMap;
	d->editBuffer->setValue( indexOf( col ),  m->property( editor ) );
	clearCellWidget( row, col );
	if ( !d->continuousEdit ) {
	    switch ( d->dat.mode() ) {
	    case QSql::Insert:
		if ( accept )
		    QTimer::singleShot( 0, this, SLOT( doInsertCurrent() ) );
		else
		    endInsert();
		break;
	    case QSql::Update:
		if ( accept )
		    QTimer::singleShot( 0, this, SLOT( doUpdateCurrent() ) );
		else
		    endUpdate();
		break;
	    default:
		break;
	    }
	}
    } else {
	setEditMode( NotEditing, -1, -1 );
    }
    if ( d->dat.mode() == QSql::None )
	viewport()->setFocus();
    updateCell( row, col );
    emit valueChanged( row, col );
}

/*! \internal */
void QDataTable::doInsertCurrent()
{
    insertCurrent();
}

/*! \internal */
void QDataTable::doUpdateCurrent()
{
    updateCurrent();
    if ( d->dat.mode() == QSql::None ) {
	viewport()->setFocus();
    }
}

/*! \reimp */
void QDataTable::activateNextCell()
{
//     if ( d->dat.mode() == QSql::None )
//	QTable::activateNextCell();
}

/*! \internal
*/

void QDataTable::endInsert()
{
    if ( d->dat.mode() != QSql::Insert )
	return;
    d->dat.setMode( QSql::None );
    d->editBuffer = 0;
    verticalHeader()->setLabel( d->editRow, QString::number( d->editRow +1 ) );
    d->editRow = -1;
    d->editCol = -1;
    d->insertRowLast = -1;
    d->insertHeaderLabelLast = QString::null;
    setEditMode( NotEditing, -1, -1 );
    setNumRows( d->insertPreRows );
    d->insertPreRows = -1;
    viewport()->setFocus();
}

/*! \internal
*/

void QDataTable::endUpdate()
{
    d->dat.setMode( QSql::None );
    d->editBuffer = 0;
    updateRow( d->editRow );
    d->editRow = -1;
    d->editCol = -1;
    setEditMode( NotEditing, -1, -1 );
}

/*!
    Protected virtual function called when editing is about to begin
    on a new record. If the table is read-only, or if there's no
    cursor or the cursor does not allow inserts, nothing happens.

    Editing takes place using the cursor's edit buffer(see
    QSqlCursor::editBuffer()).

    When editing begins, a new row is created in the table marked with
    an asterisk '*' in the row's vertical header column, i.e. at the
    left of the row.
*/

bool QDataTable::beginInsert()
{
    if ( !sqlCursor() || isReadOnly() || !numCols() )
	return FALSE;
    if ( !sqlCursor()->canInsert() )
	return FALSE;
    int i = 0;
    int row = currentRow();

    d->insertPreRows = numRows();
    if ( row < 0 || numRows() < 1 )
	row = 0;
    setNumRows( d->insertPreRows + 1 );
    setCurrentCell( row, 0 );
    d->editBuffer = sqlCursor()->primeInsert();
    emit primeInsert( d->editBuffer );
    d->dat.setMode( QSql::Insert );
    int lastRow = row;
    int lastY = contentsY() + visibleHeight();
    for ( i = row; i < numRows() ; ++i ) {
	QRect cg = cellGeometry( i, 0 );
	if ( (cg.y()+cg.height()) > lastY ) {
	    lastRow = i;
	    break;
	}
    }
    if ( lastRow == row && ( numRows()-1 > row ) )
	lastRow = numRows() - 1;
    d->insertRowLast = lastRow;
    d->insertHeaderLabelLast = verticalHeader()->label( d->insertRowLast );
    verticalHeader()->setLabel( row, "*" );
    d->editRow = row;
    // in the db world it's common to allow inserting new records
    // into a table that has read-only columns - temporarily
    // switch off read-only mode for such columns
    bool fakeReadOnly = isColumnReadOnly( 0 );
    setColumnReadOnly( 0, FALSE );
    if ( QTable::beginEdit( row, 0, FALSE ) )
	setEditMode( Editing, row, 0 );
    setColumnReadOnly( 0, fakeReadOnly );
    return TRUE;
}

/*!
    Protected virtual function called when editing is about to begin
    on an existing row. If the table is read-only, or if there's no
    cursor, nothing happens.

    Editing takes place using the cursor's edit buffer (see
    QSqlCursor::editBuffer()).

    \a row and \a col refer to the row and column in the QDataTable.

    (\a replace is provided for reimplementors and reflects the API of
    QTable::beginEdit().)
*/

QWidget* QDataTable::beginUpdate ( int row, int col, bool replace )
{
    if ( !sqlCursor() || isReadOnly() || isColumnReadOnly( col ) )
	return 0;
    setCurrentCell( row, col );
    d->dat.setMode( QSql::Update );
    if ( sqlCursor()->seek( row ) ) {
	d->editBuffer = sqlCursor()->primeUpdate();
	sqlCursor()->seek( currentRow() );
	emit primeUpdate( d->editBuffer );
	return QTable::beginEdit( row, col, replace );
    }
    return 0;
}

/*!
    For an editable table, issues an insert on the current cursor
    using the values in the cursor's edit buffer. If there is no
    current cursor or there is no current "insert" row, nothing
    happens. If confirmEdits() or confirmInsert() is TRUE,
    confirmEdit() is called to confirm the insert. Returns TRUE if the
    insert succeeded; otherwise returns FALSE.

    The underlying cursor must have a valid primary index to ensure
    that a unique record is inserted within the database otherwise the
    database may be changed to an inconsistent state.
*/

bool QDataTable::insertCurrent()
{
    if ( d->dat.mode() != QSql::Insert || ! numCols() )
	return FALSE;
    if ( !sqlCursor()->canInsert() ) {
#ifdef QT_CHECK_RANGE
	qWarning("QDataTable::insertCurrent: insert not allowed for " +
		 sqlCursor()->name() );
#endif
	endInsert();
	return FALSE;
    }
    int b = 0;
    int conf = QSql::Yes;
    if ( confirmEdits() || confirmInsert() )
	conf = confirmEdit( QSql::Insert );
    switch ( conf ) {
    case QSql::Yes: {
#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor( Qt::waitCursor );
#endif
	emit beforeInsert( d->editBuffer );
	b = sqlCursor()->insert();
#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
#endif
	if ( ( !b && !sqlCursor()->isActive() ) || !sqlCursor()->isActive() ) {
	    handleError( sqlCursor()->lastError() );
	    endInsert(); // cancel the insert if anything goes wrong
	    refresh();
	} else {
	    endInsert();
	    refresh();
	    QSqlIndex idx = sqlCursor()->primaryIndex();
	    findBuffer( idx, d->lastAt );
	    repaintContents( contentsX(), contentsY(), visibleWidth(), visibleHeight(), FALSE );
	    emit cursorChanged( QSql::Insert );
	}
	break;
    }
    case QSql::No:
	endInsert();
	break;
    case QSql::Cancel:
	if ( QTable::beginEdit( currentRow(), currentColumn(), FALSE ) )
	    setEditMode( Editing, currentRow(), currentColumn() );
	break;
    }
    return ( b > 0 );
}

/*! \internal

  Updates the row \a row.
*/

void QDataTable::updateRow( int row )
{
    for ( int i = 0; i < numCols(); ++i )
	updateCell( row, i );
}

/*!
    For an editable table, issues an update using the cursor's edit
    buffer. If there is no current cursor or there is no current
    selection, nothing happens. If confirmEdits() or confirmUpdate()
    is TRUE, confirmEdit() is called to confirm the update. Returns
    TRUE if the update succeeded; otherwise returns FALSE.

    The underlying cursor must have a valid primary index to ensure
    that a unique record is updated within the database otherwise the
    database may be changed to an inconsistent state.
*/

bool QDataTable::updateCurrent()
{
    if ( d->dat.mode() != QSql::Update )
	return FALSE;
    if ( sqlCursor()->primaryIndex().count() == 0 ) {
#ifdef QT_CHECK_RANGE
	qWarning("QDataTable::updateCurrent: no primary index for " +
		 sqlCursor()->name() );
#endif
	endUpdate();
	return FALSE;
    }
    if ( !sqlCursor()->canUpdate() ) {
#ifdef QT_CHECK_RANGE
	qWarning("QDataTable::updateCurrent: updates not allowed for " +
		 sqlCursor()->name() );
#endif
	endUpdate();
	return FALSE;
    }
    int b = 0;
    int conf = QSql::Yes;
    if ( confirmEdits() || confirmUpdate() )
	conf = confirmEdit( QSql::Update );
    switch ( conf ) {
    case QSql::Yes: {
#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor( Qt::waitCursor );
#endif
	emit beforeUpdate( d->editBuffer );
	b = sqlCursor()->update();
#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
#endif
	if ( ( !b && !sqlCursor()->isActive() ) || !sqlCursor()->isActive() ) {
	    handleError( sqlCursor()->lastError() );
	    endUpdate();
	    refresh();
	    setCurrentCell( d->editRow, d->editCol );
	    if ( QTable::beginEdit( d->editRow, d->editCol, FALSE ) )
		setEditMode( Editing, d->editRow, d->editCol );
	} else {
	    emit cursorChanged( QSql::Update );
	    refresh();
	    endUpdate();
	}
	break;
    }
    case QSql::No:
	endUpdate();
	setEditMode( NotEditing, -1, -1 );
	break;
    case QSql::Cancel:
	setCurrentCell( d->editRow, d->editCol );
	if ( QTable::beginEdit( d->editRow, d->editCol, FALSE ) )
	    setEditMode( Editing, d->editRow, d->editCol );
	break;
    }
    return ( b > 0 );
}

/*!
    For an editable table, issues a delete on the current cursor's
    primary index using the values of the currently selected row. If
    there is no current cursor or there is no current selection,
    nothing happens. If confirmEdits() or confirmDelete() is TRUE,
    confirmEdit() is called to confirm the delete. Returns TRUE if the
    delete succeeded; otherwise FALSE.

    The underlying cursor must have a valid primary index to ensure
    that a unique record is deleted within the database otherwise the
    database may be changed to an inconsistent state.
*/

bool QDataTable::deleteCurrent()
{
    if ( !sqlCursor() || isReadOnly() )
	return FALSE;
    if ( sqlCursor()->primaryIndex().count() == 0 ) {
#ifdef QT_CHECK_RANGE
	qWarning("QDataTable::deleteCurrent: no primary index " +
		 sqlCursor()->name() );
#endif
	return FALSE;
    }
    if ( !sqlCursor()->canDelete() )
	return FALSE;

    int b = 0;
    int conf = QSql::Yes;
    if ( confirmEdits() || confirmDelete() )
	conf = confirmEdit( QSql::Delete );

    // Have to have this here - the confirmEdit() might pop up a
    // dialog that causes a repaint which the cursor to the
    // record it has to repaint.
    if ( !sqlCursor()->seek( currentRow() ) )
	return FALSE;
    switch ( conf ) {
	case QSql::Yes:{
#ifndef QT_NO_CURSOR
	    QApplication::setOverrideCursor( Qt::waitCursor );
#endif
	    sqlCursor()->primeDelete();
	    emit primeDelete( sqlCursor()->editBuffer() );
	    emit beforeDelete( sqlCursor()->editBuffer() );
	    b = sqlCursor()->del();
#ifndef QT_NO_CURSOR
	    QApplication::restoreOverrideCursor();
#endif
	    if ( !b )
		handleError( sqlCursor()->lastError() );
	    refresh();
	    emit cursorChanged( QSql::Delete );
	    setCurrentCell( currentRow(), currentColumn() );
	    repaintContents( contentsX(), contentsY(), visibleWidth(), visibleHeight(), FALSE );
	    verticalHeader()->repaint(); // get rid of trailing garbage
	}
	break;
    case QSql::No:
	setEditMode( NotEditing, -1, -1 );
	break;
    }
    return ( b > 0 );
}

/*!
    Protected virtual function which returns a confirmation for an
    edit of mode \a m. Derived classes can reimplement this function
    to provide their own confirmation dialog. The default
    implementation uses a message box which prompts the user to
    confirm the edit action.
*/

QSql::Confirm QDataTable::confirmEdit( QSql::Op m )
{
    return d->dat.confirmEdit( this, m );
}

/*!
    Protected virtual function which returns a confirmation for
    cancelling an edit mode of \a m. Derived classes can reimplement
    this function to provide their own cancel dialog. The default
    implementation uses a message box which prompts the user to
    confirm the cancel.
*/

QSql::Confirm  QDataTable::confirmCancel( QSql::Op m )
{
    return d->dat.confirmCancel( this, m );
}


/*!
    Searches the current cursor for a cell containing the string \a
    str starting at the current cell and working forwards (or
    backwards if \a backwards is TRUE). If the string is found, the
    cell containing the string is set as the current cell. If \a
    caseSensitive is FALSE the case of \a str will be ignored.

    The search will wrap, i.e. if the first (or if backwards is TRUE,
    last) cell is reached without finding \a str the search will
    continue until it reaches the starting cell. If \a str is not
    found the search will fail and the current cell will remain
    unchanged.
*/
void QDataTable::find( const QString & str, bool caseSensitive, bool backwards )
{
    if ( !sqlCursor() )
	return;

    QSqlCursor * r = sqlCursor();
    QString tmp, text;
    uint  row = currentRow(), startRow = row,
	  col = backwards ? currentColumn() - 1 : currentColumn() + 1;
    bool  wrap = TRUE, found = FALSE;

    if( str.isEmpty() || str.isNull() )
	return;

    if( !caseSensitive )
	tmp = str.lower();
    else
	tmp = str;

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor( Qt::waitCursor );
#endif
    while( wrap ){
	while( !found && r->seek( row ) ){
	    for( int i = col; backwards ? (i >= 0) : (i < (int) numCols());
		 backwards ? i-- : i++ )
	    {
		text = r->value( indexOf( i ) ).toString();
		if( !caseSensitive ){
		    text = text.lower();
		}
		if( text.contains( tmp ) ){
		    setCurrentCell( row, i );
		    col = i;
		    found = TRUE;
		}
	    }
	    if( !backwards ){
		col = 0;
		row++;
	    } else {
		col = numCols() - 1;
		row--;
	    }
	}
	if( !backwards ){
	    if( startRow != 0 ){
		startRow = 0;
	    } else {
		wrap = FALSE;
	    }
	    r->first();
	    row = 0;
	} else {
	    if( startRow != (uint) (numRows() - 1) ){
		startRow = numRows() - 1;
	    } else {
		wrap = FALSE;
	    }
	    r->last();
	    row = numRows() - 1;
	}
    }
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
}


/*!
    Resets the table so that it displays no data.

    \sa setSqlCursor()
*/

void QDataTable::reset()
{
    clearCellWidget( currentRow(), currentColumn() );
    switch ( d->dat.mode() ) {
    case QSql::Insert:
	endInsert();
	break;
    case QSql::Update:
	endUpdate();
	break;
    default:
	break;
    }
    ensureVisible( 0, 0 );
    verticalScrollBar()->setValue(0);
    setNumRows(0);

    d->haveAllRows = FALSE;
    d->continuousEdit = FALSE;
    d->dat.setMode( QSql::None );
    d->editRow = -1;
    d->editCol = -1;
    d->insertRowLast = -1;
    d->insertHeaderLabelLast = QString::null;
    d->cancelMode = FALSE;
    d->lastAt = -1;
    d->fld.clear();
    d->fldLabel.clear();
    d->fldWidth.clear();
    d->fldIcon.clear();
    d->fldHidden.clear();
    if ( sorting() )
	horizontalHeader()->setSortIndicator( -1 );
}

/*!
    Returns the index of the field within the current SQL query that
    is displayed in column \a i.
*/

int QDataTable::indexOf( uint i ) const
{
    QDataTablePrivate::ColIndex::ConstIterator it = d->colIndex.at( i );
    if ( it != d->colIndex.end() )
	return *it;
    return -1;
}

/*!
    Returns TRUE if the table will automatically delete the cursor
    specified by setSqlCursor(); otherwise returns FALSE.
*/

bool QDataTable::autoDelete() const
{
    return d->cur.autoDelete();
}

/*!
    Sets the cursor auto-delete flag to \a enable. If \a enable is
    TRUE, the table will automatically delete the cursor specified by
    setSqlCursor(). If \a enable is FALSE (the default), the cursor
    will not be deleted.
*/

void QDataTable::setAutoDelete( bool enable )
{
    d->cur.setAutoDelete( enable );
}

/*!
    \property QDataTable::autoEdit
    \brief whether the data table automatically applies edits

    The default value for this property is TRUE. When the user begins
    an insert or update in the table there are two possible outcomes
    when they navigate to another record:

    \list 1
    \i the insert or update is is performed -- this occurs if autoEdit is TRUE
    \i the insert or update is abandoned -- this occurs if autoEdit is FALSE
    \endlist
*/

void QDataTable::setAutoEdit( bool autoEdit )
{
    d->dat.setAutoEdit( autoEdit );
}

bool QDataTable::autoEdit() const
{
    return d->dat.autoEdit();
}

/*!
    \property QDataTable::nullText
    \brief the text used to represent NULL values

    The nullText property will be used to represent NULL values in the
    table. The default value is provided by the cursor's driver.
*/

void QDataTable::setNullText( const QString& nullText )
{
    d->nullTxt = nullText;
    d->nullTxtChanged = TRUE;
}

QString QDataTable::nullText() const
{
    return d->nullTxt;
}

/*!
    \property QDataTable::trueText
    \brief the text used to represent true values

    The trueText property will be used to represent NULL values in the
    table. The default value is "True".
*/

void QDataTable::setTrueText( const QString& trueText )
{
    d->trueTxt = trueText;
}

QString QDataTable::trueText() const
{
    return d->trueTxt;
}

/*!
    \property QDataTable::falseText
    \brief the text used to represent false values

    The falseText property will be used to represent NULL values in
    the table. The default value is "False".
*/

void QDataTable::setFalseText( const QString& falseText )
{
    d->falseTxt = falseText;
}

QString QDataTable::falseText() const
{
    return d->falseTxt;
}

/*!
    \property QDataTable::dateFormat
    \brief the format used for displaying date/time values

    The dateFormat property is used for displaying date/time values in
    the table. The default value is \c Qt::LocalDate.
*/

void QDataTable::setDateFormat( const DateFormat f )
{
    d->datefmt = f;
}

Qt::DateFormat QDataTable::dateFormat() const
{
    return d->datefmt;
}

/*!
    \property QDataTable::numRows

    \brief the number of rows in the table
*/

int QDataTable::numRows() const
{
    return QTable::numRows();
}

/*!
    \reimp

    The number of rows in the table will be determined by the cursor
    (see setSqlCursor()), so normally this function should never be
    called. It is included for completeness.
*/

void QDataTable::setNumRows ( int r )
{
    QTable::setNumRows( r );
}

/*!
    \reimp

    The number of columns in the table will be determined
    automatically (see addColumn()), so normally this function should
    never be called. It is included for completeness.
*/

void QDataTable::setNumCols ( int r )
{
    QTable::setNumCols( r );
}

/*!
    \property QDataTable::numCols

    \brief the number of columns in the table
*/

int QDataTable::numCols() const
{
    return QTable::numCols();
}

/*!
    Returns the text in cell \a row, \a col, or an empty string if the
    cell is empty. If the cell's value is NULL then nullText() will be
    returned. If the cell does not exist then QString::null is
    returned.
*/

QString QDataTable::text ( int row, int col ) const
{
    if ( !sqlCursor() )
	return QString::null;

    QString s;
    if ( sqlCursor()->seek( row ) )
	s = sqlCursor()->value( indexOf( col ) ).toString();
    sqlCursor()->seek( currentRow() );
    return s;
}

/*!
    Returns the value in cell \a row, \a col, or an invalid value if
    the cell does not exist or has no value.
*/

QVariant QDataTable::value ( int row, int col ) const
{
    if ( !sqlCursor() )
	return QVariant();

    QVariant v;
    if ( sqlCursor()->seek( row ) )
	v = sqlCursor()->value( indexOf( col ) );
    sqlCursor()->seek( currentRow() );
    return v;
}

/*!  \internal
  Used to update the table when the size of the result set cannot be
  determined - divide the result set into pages and load the pages as
  the user moves around in the table.
*/
void QDataTable::loadNextPage()
{
    if ( d->haveAllRows )
	return;
    if ( !sqlCursor() )
	return;
    int pageSize = 0;
    int lookAhead = 0;
    if ( height() ) {
	pageSize = (int)( height() * 2 / 20 );
	lookAhead = pageSize / 2;
    }
    int startIdx = verticalScrollBar()->value() / 20;
    int endIdx = startIdx + pageSize + lookAhead;
    if ( endIdx < numRows() || endIdx < 0 )
	return;

    // check for empty result set
    if ( sqlCursor()->at() == QSql::BeforeFirst && !sqlCursor()->next() ) {
	d->haveAllRows = TRUE;
	return;
    }

    while ( endIdx > 0 && !sqlCursor()->seek( endIdx ) )
	endIdx--;
    if ( endIdx != ( startIdx + pageSize + lookAhead ) )
	d->haveAllRows = TRUE;
    // small hack to prevent QTable from moving the view when a row
    // is selected and the contents is resized
    SelectionMode m = selectionMode();
    clearSelection();
    setSelectionMode( NoSelection );
    setNumRows( endIdx + 1 );
    sqlCursor()->seek( currentRow() );
    setSelectionMode( m );
}

/*! \internal */
void QDataTable::sliderPressed()
{
    disconnect( verticalScrollBar(), SIGNAL( valueChanged(int) ),
		this, SLOT( loadNextPage() ) );
}

/*! \internal */
void QDataTable::sliderReleased()
{
    loadNextPage();
    connect( verticalScrollBar(), SIGNAL( valueChanged(int) ),
	     this, SLOT( loadNextPage() ) );
}

/*!
    Sorts column \a col in ascending order if \a ascending is TRUE
    (the default); otherwise sorts in descending order.

    The \a wholeRows parameter is ignored; QDataTable always sorts
    whole rows by the specified column.
*/

void QDataTable::sortColumn ( int col, bool ascending,
			      bool  )
{
    if ( sorting() ) {
	if ( isEditing() && d->dat.mode() != QSql::None )
	    endEdit( d->editRow, d->editCol, autoEdit(), FALSE );
	if ( !sqlCursor() )
	    return;
	QSqlIndex lastSort = sqlCursor()->sort();
	QSqlIndex newSort( lastSort.cursorName(), "newSort" );
	QSqlField *field = sqlCursor()->field( indexOf( col ) );
	if ( field )
	    newSort.append( *field );
	newSort.setDescending( 0, !ascending );
	horizontalHeader()->setSortIndicator( col, ascending );
	setSort( newSort );
	refresh();
    }
}

/*! \reimp */
void QDataTable::columnClicked ( int col )
{
    if ( sorting() ) {
	if ( !sqlCursor() )
	    return;
	QSqlIndex lastSort = sqlCursor()->sort();
	bool asc = TRUE;
	if ( lastSort.count() && lastSort.field( 0 )->name() == sqlCursor()->field( indexOf( col ) )->name() )
	    asc = lastSort.isDescending( 0 );
	sortColumn( col, asc );
	emit currentChanged( sqlCursor() );
    }
}

/*!
    \reimp

    Repaints the cell at \a row, \a col.
*/
void QDataTable::repaintCell( int row, int col )
{
    QRect cg = cellGeometry( row, col );
    QRect re( QPoint( cg.x() - 2, cg.y() - 2 ),
	      QSize( cg.width() + 4, cg.height() + 4 ) );
    repaintContents( re, FALSE );
}

/*!
    \reimp

    This function renders the cell at \a row, \a col with the value of
    the corresponding cursor field on the painter \a p. Depending on
    the table's current edit mode, paintField() is called for the
    appropriate cursor field. \a cr describes the cell coordinates in
    the content coordinate system. If \a selected is TRUE the cell has
    been selected and would normally be rendered differently than an
    unselected cell.

    \sa QSql::isNull()
*/

void QDataTable::paintCell( QPainter * p, int row, int col, const QRect & cr,
			  bool selected, const QColorGroup &cg )
{
    QTable::paintCell( p, row, col, cr, selected, cg );  // empty cell

    if ( !sqlCursor() )
	return;

    p->setPen( selected ? cg.highlightedText() : cg.text() );
    if ( d->dat.mode() != QSql::None ) {
	if ( row == d->editRow && d->editBuffer ) {
	    paintField( p, d->editBuffer->field( indexOf( col ) ), cr,
			selected );
	} else if ( row > d->editRow && d->dat.mode() == QSql::Insert ) {
	    if ( sqlCursor()->seek( row - 1 ) )
		paintField( p, sqlCursor()->field( indexOf( col ) ), cr,
			    selected );
	} else {
	    if ( sqlCursor()->seek( row ) )
		paintField( p, sqlCursor()->field( indexOf( col ) ), cr,
			    selected );
	}
    } else {
	if ( sqlCursor()->seek( row ) )
		paintField( p, sqlCursor()->field( indexOf( col ) ), cr, selected );

    }
}


/*!
    Paints the \a field on the painter \a p. The painter has already
    been translated to the appropriate cell's origin where the \a
    field is to be rendered. \a cr describes the cell coordinates in
    the content coordinate system. The \a selected parameter is
    ignored.

    If you want to draw custom field content you must reimplement
    paintField() to do the custom drawing. The default implementation
    renders the \a field value as text. If the field is NULL,
    nullText() is displayed in the cell. If the field is Boolean,
    trueText() or falseText() is displayed as appropriate.
*/

void QDataTable::paintField( QPainter * p, const QSqlField* field,
			    const QRect & cr, bool )
{
    if ( !field )
	return;
    p->drawText( 2,2, cr.width()-4, cr.height()-4, fieldAlignment( field ), fieldToString( field ) );
}

/*!
    Returns the alignment for \a field.
*/

int QDataTable::fieldAlignment( const QSqlField* /*field*/ )
{
    return Qt::AlignLeft | Qt::AlignVCenter; //## Reggie: add alignment to QTable
}


/*!
    If the cursor's \a sql driver supports query sizes, the number of
    rows in the table is set to the size of the query. Otherwise, the
    table dynamically resizes itself as it is scrolled. If \a sql is
    not active, it is made active by issuing a select() on the cursor
    using the \a sql cursor's current filter and current sort.
*/

void QDataTable::setSize( QSqlCursor* sql )
{
    // ### what are the connect/disconnect calls doing here!? move to refresh()
    if ( sql->driver() && sql->driver()->hasFeature( QSqlDriver::QuerySize ) ) {
	setVScrollBarMode( Auto );
 	disconnect( verticalScrollBar(), SIGNAL( sliderPressed() ),
		    this, SLOT( sliderPressed() ) );
 	disconnect( verticalScrollBar(), SIGNAL( sliderReleased() ),
		    this, SLOT( sliderReleased() ) );
	disconnect( verticalScrollBar(), SIGNAL( valueChanged(int) ),
		    this, SLOT( loadNextPage() ) );
	if ( numRows() != sql->size() )
	    setNumRows( sql->size() );
    } else {
	setVScrollBarMode( AlwaysOn );
 	connect( verticalScrollBar(), SIGNAL( sliderPressed() ),
 		 this, SLOT( sliderPressed() ) );
 	connect( verticalScrollBar(), SIGNAL( sliderReleased() ),
 		 this, SLOT( sliderReleased() ) );
	connect( verticalScrollBar(), SIGNAL( valueChanged(int) ),
		 this, SLOT( loadNextPage() ) );
	setNumRows(0);
	loadNextPage();
    }
}

/*!
    Sets \a cursor as the data source for the table. To force the
    display of the data from \a cursor, use refresh(). If \a
    autoPopulate is TRUE, columns are automatically created based upon
    the fields in the \a cursor record. If \a autoDelete is TRUE (the
    default is FALSE), the table will take ownership of the \a cursor
    and delete it when appropriate. If the \a cursor is read-only, the
    table becomes read-only. The table adopts the cursor's driver's
    definition for representing NULL values as strings.

    \sa refresh() setReadOnly() setAutoDelete() QSqlDriver::nullText()
*/

void QDataTable::setSqlCursor( QSqlCursor* cursor, bool autoPopulate, bool autoDelete )
{
    setUpdatesEnabled( FALSE );
    d->cur.setCursor( 0 );
    if ( cursor ) {
	d->cur.setCursor( cursor, autoDelete );
	if ( autoPopulate ) {
	    d->fld.clear();
	    d->fldLabel.clear();
	    d->fldWidth.clear();
	    d->fldIcon.clear();
	    d->fldHidden.clear();
	    for ( uint i = 0; i < sqlCursor()->count(); ++i ) {
		addColumn( sqlCursor()->field( i )->name(), sqlCursor()->field( i )->name() );
		setColumnReadOnly( i, sqlCursor()->field( i )->isReadOnly() );
	    }
	}
	setReadOnly( sqlCursor()->isReadOnly() );
	if ( sqlCursor()->driver() && !d->nullTxtChanged )
	    setNullText(sqlCursor()->driver()->nullText() );
	setAutoDelete( autoDelete );
    } else {
	setNumRows( 0 );
	setNumCols( 0 );
    }
    setUpdatesEnabled( TRUE );
}


/*!
    Protected virtual function which is called when an error \a e has
    occurred on the current cursor(). The default implementation
    displays a warning message to the user with information about the
    error.
*/
void QDataTable::handleError( const QSqlError& e )
{
    d->dat.handleError( this, e );
}

/*! \reimp
  */

void QDataTable::keyPressEvent( QKeyEvent* e )
{
    switch( e->key() ) {
    case Key_Left:
    case Key_Right:
    case Key_Up:
    case Key_Down:
    case Key_Prior:
    case Key_Next:
    case Key_Home:
    case Key_End:
    case Key_F2:
    case Key_Enter: case Key_Return:
    case Key_Tab: case Key_BackTab:
	QTable::keyPressEvent( e );
    default:
	return;
    }
}

/*!  \reimp
*/

void QDataTable::resizeData ( int )
{

}

/*!  \reimp
*/

QTableItem * QDataTable::item ( int, int ) const
{
    return 0;
}

/*!  \reimp
*/

void QDataTable::setItem ( int , int , QTableItem * )
{

}

/*!  \reimp
*/

void QDataTable::clearCell ( int , int )
{

}

/*!  \reimp
*/

void QDataTable::setPixmap ( int , int , const QPixmap &  )
{

}

/*! \reimp */
void QDataTable::takeItem ( QTableItem * )
{

}

/*!
    Installs a new SQL editor factory \a f. This enables the user to
    create and instantiate their own editors for use in cell editing.
    Note that QDataTable takes ownership of this pointer, and will
    delete it when it is no longer needed or when
    installEditorFactory() is called again.

    \sa QSqlEditorFactory
*/

void QDataTable::installEditorFactory( QSqlEditorFactory * f )
{
    if( f ) {
	delete d->editorFactory;
	d->editorFactory = f;
    }
}

/*!
    Installs a new property map \a m. This enables the user to create
    and instantiate their own property maps for use in cell editing.
    Note that QDataTable takes ownership of this pointer, and will
    delete it when it is no longer needed or when installPropertMap()
    is called again.

    \sa QSqlPropertyMap
*/

void QDataTable::installPropertyMap( QSqlPropertyMap* m )
{
    if ( m ) {
	delete d->propertyMap;
	d->propertyMap = m;
    }
}

/*!  \internal

  Sets the current selection to \a row, \a col.
*/

void QDataTable::setCurrentSelection( int row, int )
{
    if ( !sqlCursor() )
	return;
    if ( row == d->lastAt )
	return;
    if ( !sqlCursor()->seek( row ) )
	return;
    d->lastAt = row;
    emit currentChanged( sqlCursor() );
}

void QDataTable::updateCurrentSelection()
{
    setCurrentSelection( currentRow(), -1 );
}

/*!
    Returns the currently selected record, or 0 if there is no current
    selection. The table owns the pointer, so do \e not delete it or
    otherwise modify it or the cursor it points to.
*/

QSqlRecord* QDataTable::currentRecord() const
{
    if ( !sqlCursor() || currentRow() < 0 )
	return 0;
    if ( !sqlCursor()->seek( currentRow() ) )
	return 0;
    return sqlCursor();
}

/*!
    Sorts column \a col in ascending order.

    \sa setSorting()
*/

void QDataTable::sortAscending( int col )
{
    sortColumn( col, TRUE );
}

/*!
    Sorts column \a col in descending order.

    \sa setSorting()
*/

void QDataTable::sortDescending( int col )
{
    sortColumn( col, FALSE );
}

/*!
    \overload void QDataTable::refresh( Refresh mode )

    Refreshes the table. If there is no currently defined cursor (see
    setSqlCursor()), nothing happens. The \a mode parameter determines
    which type of refresh will take place.

    \sa Refresh setSqlCursor() addColumn()
*/

void QDataTable::refresh( QDataTable::Refresh mode )
{
    QSqlCursor* cur = sqlCursor();
    if ( !cur )
	return;
    bool refreshData = ( (mode & RefreshData) == RefreshData );
    bool refreshCol = ( (mode & RefreshColumns) == RefreshColumns );
    if ( ( (mode & RefreshAll) == RefreshAll ) ) {
	refreshData = TRUE;
	refreshCol = TRUE;
    }
    if ( !refreshCol && d->fld.count() && numCols() == 0 )
	refreshCol = TRUE;
    viewport()->setUpdatesEnabled( FALSE );
    d->haveAllRows = FALSE;
    if ( refreshData ) {
	if ( !d->cur.refresh() && d->cur.cursor() ) {
	    handleError( d->cur.cursor()->lastError() );
	}
	d->lastAt = -1;
    }
    if ( refreshCol ) {
	setNumCols( 0 );
	d->colIndex.clear();
	if ( d->fld.count() ) {
	    QSqlField* field = 0;
	    int i;
	    int fpos = -1;
	    for ( i = 0; i < (int)d->fld.count(); ++i ) {
		if ( cur->field( i ) && cur->field( i )->name() == d->fld[ i ] )
		    // if there is a field with the desired name on the desired position
		    // then we take that
		    fpos = i;
		else
		    // otherwise we take the first field that matches the desired name
		    fpos = cur->position( d->fld[ i ] );
		field = cur->field( fpos );
		if ( field && ( cur->isGenerated( fpos ) ||
				cur->isCalculated( field->name() ) ) )
		{
		    setNumCols( numCols() + 1 );
		    d->colIndex.append( fpos );
		    setColumnReadOnly( numCols()-1, field->isReadOnly() || isColumnReadOnly( numCols()-1 ) );
		    horizontalHeader()->setLabel( numCols()-1, d->fldIcon[ i ], d->fldLabel[ i ] );
		    if ( d->fldHidden[ i ] ) {
			QTable::showColumn( i ); // ugly but necessary
			QTable::hideColumn( i );
		    } else {
			QTable::showColumn( i );
		    }
		    if ( d->fldWidth[ i ] > -1 )
			QTable::setColumnWidth( i, d->fldWidth[i] );
		}
	    }
	}
    }
    viewport()->setUpdatesEnabled( TRUE );
    viewport()->repaint( FALSE );
    horizontalHeader()->repaint();
    verticalHeader()->repaint();
    setSize( cur );
    // keep others aware
    if ( d->lastAt == -1 )
 	setCurrentSelection( -1, -1 );
    else if ( d->lastAt != currentRow() )
	setCurrentSelection( currentRow(), currentColumn() );
    if ( cur->isValid() )
	emit currentChanged( sqlCursor() );
}

/*!
    Refreshes the table. The cursor is refreshed using the current
    filter, the current sort, and the currently defined columns.
    Equivalent to calling refresh( QDataTable::RefreshData ).
*/

void QDataTable::refresh()
{
    refresh( RefreshData );
}

/*!
    \reimp

    Selects the record in the table using the current cursor edit
    buffer and the fields specified by the index \a idx. If \a atHint
    is specified, it will be used as a hint about where to begin
    searching.
*/

bool QDataTable::findBuffer( const QSqlIndex& idx, int atHint )
{
    QSqlCursor* cur = sqlCursor();
    if ( !cur )
	return FALSE;
    bool found = d->cur.findBuffer( idx, atHint );
    if ( found )
	setCurrentCell( cur->at(), currentColumn() );
    return found;
}

/*! \internal
    Returns the string representation of a database field.
*/
QString QDataTable::fieldToString( const QSqlField * field )
{
    QString text;
    if ( field->isNull() ) {
	text = nullText();
    } else {
	QVariant val = field->value();
	switch ( val.type() ) {
	    case QVariant::Bool:
		text = val.toBool() ? d->trueTxt : d->falseTxt;
		break;
	    case QVariant::Date:
		text = val.toDate().toString( d->datefmt );
		break;
	    case QVariant::Time:
		text = val.toTime().toString( d->datefmt );
		break;
	    case QVariant::DateTime:
		text = val.toDateTime().toString( d->datefmt );
		break;
	    default:
		text = val.toString();
		break;
	}
    }
    return text;
}

/*!
    \reimp
*/

void QDataTable::swapColumns( int col1, int col2, bool )
{
    QString fld = d->fld[ col1 ];
    QString fldLabel = d->fldLabel[ col1 ];
    QIconSet fldIcon = d->fldIcon[ col1 ];
    int fldWidth = d->fldWidth[ col1 ];

    d->fld[ col1 ] = d->fld[ col2 ];
    d->fldLabel[ col1 ] = d->fldLabel[ col2 ];
    d->fldIcon[ col1 ] = d->fldIcon[ col2 ];
    d->fldWidth[ col1 ] = d->fldWidth[ col2 ];

    d->fld[ col2 ] = fld;
    d->fldLabel[ col2 ] = fldLabel;
    d->fldIcon[ col2 ] = fldIcon;
    d->fldWidth[ col2 ] = fldWidth;

    int colIndex = d->colIndex[ col1 ];
    d->colIndex[ col1 ] = d->colIndex[ col2 ];
    d->colIndex[ col2 ] = colIndex;
}

/*!
    \reimp
*/

void QDataTable::drawContents( QPainter * p, int cx, int cy, int cw, int ch )
{
    QTable::drawContents( p, cx, cy, cw, ch );
    if ( sqlCursor() && currentRow() >= 0 )
	sqlCursor()->seek( currentRow() );
}

/*!
    \reimp
*/

void QDataTable::hideColumn( int col )
{
    d->fldHidden[col] = TRUE;
    refresh( RefreshColumns );
}

/*!
    \reimp
*/

void QDataTable::showColumn( int col )
{
    d->fldHidden[col] = FALSE;
    refresh( RefreshColumns );
}

/*!
    \fn void QDataTable::currentChanged( QSqlRecord* record )

    This signal is emitted whenever a new row is selected in the
    table. The \a record parameter points to the contents of the newly
    selected record.
*/

/*!
    \fn void QDataTable::primeInsert( QSqlRecord* buf )

    This signal is emitted after the cursor is primed for insert by
    the table, when an insert action is beginning on the table. The \a
    buf parameter points to the edit buffer being inserted. Connect to
    this signal in order to, for example, prime the record buffer with
    default data values.
*/

/*!
    \fn void QDataTable::primeUpdate( QSqlRecord* buf )

    This signal is emitted after the cursor is primed for update by
    the table, when an update action is beginning on the table. The \a
    buf parameter points to the edit buffer being updated. Connect to
    this signal in order to, for example, provide some visual feedback
    that the user is in 'edit mode'.
*/

/*!
    \fn void QDataTable::primeDelete( QSqlRecord* buf )

    This signal is emitted after the cursor is primed for delete by
    the table, when a delete action is beginning on the table. The \a
    buf parameter points to the edit buffer being deleted. Connect to
    this signal in order to, for example, record auditing information
    on deletions.
*/

/*!
    \fn void QDataTable::beforeInsert( QSqlRecord* buf )

    This signal is emitted just before the cursor's edit buffer is
    inserted into the database. The \a buf parameter points to the
    edit buffer being inserted. Connect to this signal to, for
    example, populate a key field with a unique sequence number.
*/

/*!
    \fn void QDataTable::beforeUpdate( QSqlRecord* buf )

    This signal is emitted just before the cursor's edit buffer is
    updated in the database. The \a buf parameter points to the edit
    buffer being updated. Connect to this signal when you want to
    transform the user's data behind-the-scenes.
*/

/*!
    \fn void QDataTable::beforeDelete( QSqlRecord* buf )

    This signal is emitted just before the currently selected record
    is deleted from the database. The \a buf parameter points to the
    edit buffer being deleted. Connect to this signal to, for example,
    copy some of the fields for later use.
*/

/*!
    \fn void QDataTable::cursorChanged( QSql::Op mode )

    This signal is emitted whenever the cursor record was changed due
    to an edit. The \a mode parameter is the type of edit that just
    took place.
*/

#endif
