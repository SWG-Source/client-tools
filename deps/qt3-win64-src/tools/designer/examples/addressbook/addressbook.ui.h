/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include "addressdetails.h"
#include "search.h"
#include <qfile.h>
#include <qtextstream.h>
#include <qfiledialog.h>
#include <qapplication.h>

void AddressBook::fileNew()
{
    AddressDetails dia( this, 0, TRUE );
    connect( &dia, SIGNAL( newAddress( const QString &,
				       const QString &,
				       const QString &,
				       const QString &,
				       const QString &,
				       const QString &,
				       const QString & ) ),
	     this, SLOT( insertAddress( const QString &,
					const QString &,
					const QString &,
					const QString &,
					const QString &,
					const QString &,
					const QString & ) ) );
    dia.exec();
}

void AddressBook::insertAddress( const QString &firstName,
				 const QString &lastName, 
				 const QString &phoneNumber,
				 const QString &street,
				 const QString &city,
				 const QString &country,
				 const QString &zipCode )
{
    QListViewItem *i = new QListViewItem( addressView );
    i->setText( 0, firstName );
    i->setText( 1, lastName );
    i->setText( 2, phoneNumber );
    i->setText( 3, street );
    i->setText( 4, city );
    i->setText( 5, country );
    i->setText( 6, zipCode );
}

void AddressBook::deleteAddress()
{
    QListViewItemIterator it( addressView );
    while ( it.current() ) {
	QListViewItem *i = it.current();
	++it;
	if ( i->isSelected() )
	    delete i;
    }
}

void AddressBook::fileOpen()
{
    addressView->clear();
    QString fn = QFileDialog::getOpenFileName();
    if ( fn.isEmpty() )
	return;
    currentFileName = fn;
    QFile f( currentFileName );
    if ( !f.open( IO_ReadOnly ) )
	return;
    QTextStream ts( &f );
    
    while ( !ts.eof() ) {
	QListViewItem *item = new QListViewItem( addressView );
	for ( int i = 0; i < 7; ++i )
	    item->setText( i, ts.readLine() );
    }
	    
}

void AddressBook::fileSave()
{
    if ( currentFileName.isEmpty() )
	fileSaveAs();
    QFile f( currentFileName );
    if ( !f.open( IO_WriteOnly ) )
	return;
    QTextStream ts( &f );
    
    QListViewItemIterator it( addressView );
    while ( it.current() ) {
	for ( int i = 0; i < 7; ++i )
	    ts << it.current()->text( i ) << endl;
	++it;
    }
    
    f.close();
}

void AddressBook::fileSaveAs()
{
    QString fn = QFileDialog::getSaveFileName();
    if ( fn.isEmpty() )
	return;
    currentFileName = fn;
    fileSave();
}

void AddressBook::fileExit()
{
    qApp->exit();
}

void AddressBook::editFind()
{
    SearchDialog dia( this, 0, TRUE );
    connect( &dia, SIGNAL( searchAddress( const QString & ) ),
	     this, SLOT( searchAddress( const QString & ) ) );
    dia.exec();
}

void AddressBook::searchAddress( const QString &expr )
{
    addressView->clearSelection();
    QListViewItemIterator it( addressView );
    while ( it.current() ) {
	for ( int i = 0; i < 7; ++i ) {
	    if ( it.current()->text( i ).find( expr ) != -1 )
		addressView->setSelected( it.current(), TRUE );
	}
	++it;
    }  
}