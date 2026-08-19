/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/
#include <qsqldriver.h>
#include <qmessagebox.h>
#include <qsqldatabase.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qsqlerror.h>
#include <qsqlcursor.h>
#include <qsqlselectcursor.h>
#include <qdatatable.h>
#include "connect.h"

static void showError( const QSqlError& err, QWidget* parent = 0 )
{
   QString errStr ( "The database reported an error\n" );
    if ( !err.databaseText().isEmpty() )
	errStr += err.databaseText();
    if ( !err.driverText().isEmpty() )
	errStr += err.driverText();
    QMessageBox::warning( parent, "Error", errStr );
}

ConnectDialog* conDiag = 0;

void SqlEx::init()
{
    hsplit->setResizeMode( lv, QSplitter::KeepSize );
    vsplit->setResizeMode( gb, QSplitter::KeepSize );
    submitBtn->setEnabled( FALSE );
    conDiag = new ConnectDialog( this, "Connection Dialog", TRUE );    
}

void SqlEx::dbConnect()
{ 
    if ( conDiag->exec() != QDialog::Accepted )
	return;
    if ( dt->sqlCursor() ) {
	dt->setSqlCursor( 0 );
    }
    // close old connection (if any)
    if ( QSqlDatabase::contains( "SqlEx" ) ) {
	QSqlDatabase* oldDb = QSqlDatabase::database( "SqlEx" );
	oldDb->close();
	QSqlDatabase::removeDatabase( "SqlEx" );
    }
    // open the new connection
    QSqlDatabase* db = QSqlDatabase::addDatabase( conDiag->comboDriver->currentText(), "SqlEx" );
    if ( !db ) {
	QMessageBox::warning( this, "Error", "Could not open database" );	
	return;
    }
    db->setHostName( conDiag->editHostname->text() );
    db->setDatabaseName( conDiag->editDatabase->text() );
    db->setPort( conDiag->portSpinBox->value() );
    if ( !db->open( conDiag->editUsername->text(), conDiag->editPassword->text() ) ) {
	showError( db->lastError(), this );
	return;
    }
    lbl->setText( "Double-Click on a table-name to view the contents" );
    lv->clear();
    
    QStringList tables = db->tables();
    for ( QStringList::Iterator it = tables.begin(); it != tables.end(); ++it ) {
	QListViewItem* lvi = new QListViewItem( lv, *it );
	QSqlRecordInfo ri = db->recordInfo ( *it );
	for ( QSqlRecordInfo::Iterator it = ri.begin(); it != ri.end(); ++it ) {
	    QString req;
	    if ( (*it).isRequired() > 0 ) {
		req = "Yes";
	    } else if ( (*it).isRequired() == 0 ) {
		req = "No";
	    } else {
		req = "?";
	    }
	    QListViewItem* fi = new QListViewItem( lvi, (*it).name(),  + QVariant::typeToName( (*it).type() ), req );
	    lvi->insertItem( fi );
	}
	lv->insertItem( lvi );	
    }
    submitBtn->setEnabled( TRUE );
}

void SqlEx::execQuery()
{
    // use a custom cursor to populate the data table
    QSqlSelectCursor* cursor = new QSqlSelectCursor( te->text(), QSqlDatabase::database( "SqlEx", TRUE ) );
    if ( cursor->isSelect() ) {
	dt->setSqlCursor( cursor, TRUE, TRUE );
	dt->setSort( QStringList() );
	dt->refresh( QDataTable::RefreshAll );
	QString txt( "Query OK" );
	if ( cursor->size() >= 0 )
	    txt += ", returned rows: " + QString::number( cursor->size() );
	lbl->setText( txt );
    } else {
	// an error occured if the cursor is not active
	if ( !cursor->isActive() ) {
	    showError( cursor->lastError(), this );
	} else {
	    lbl->setText( QString("Query OK, affected rows: %1").arg( cursor->numRowsAffected() ) );
	}
    }
}

void SqlEx::showTable( QListViewItem * item )
{
    // get the table name
    QListViewItem* i = item->parent();
    if ( !i ) {
	i = item;
    }

    // populate the data table
    QSqlCursor* cursor = new QSqlCursor( i->text( 0 ), TRUE, QSqlDatabase::database( "SqlEx", TRUE ) );
    dt->setSqlCursor( cursor, TRUE, TRUE );
    dt->setSort( cursor->primaryIndex() );
    dt->refresh( QDataTable::RefreshAll );
    lbl->setText( "Displaying table " + i->text( 0 ) );
}
