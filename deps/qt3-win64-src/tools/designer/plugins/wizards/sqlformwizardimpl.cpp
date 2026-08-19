 /**********************************************************************
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Designer.
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
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "sqlformwizardimpl.h"

#include <qlistbox.h>
#include <qwidget.h>
#include <qcheckbox.h>
#include <qlineedit.h>

#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qlistview.h>
#include <qfeatures.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <limits.h>

#ifndef QT_NO_SQL
#include <qdatatable.h>
#include <qdatabrowser.h>
#include <qdataview.h>
#include <qsqleditorfactory.h>
#include <qsqlindex.h>
#include <qsqlcursor.h>
#endif

SqlFormWizard::SqlFormWizard( QUnknownInterface *aIface, QWidget *w,
			      QWidget* parent, DesignerFormWindow *fw, const char* name, bool modal, WFlags fl )
    : SqlFormWizardBase( parent, name, modal, fl ), widget( w ), appIface( aIface ),
     mode( None )
{
    appIface->addRef();
    formWindow = fw;
    setFinishEnabled( finishPage, TRUE );

    /* set mode of operation */
    if ( ::qt_cast<QDataTable*>(widget) ) {
	setCaption( "Data Table Wizard" );
	mode = Table;
	setAppropriate( navigPage, FALSE );
	setAppropriate( layoutPage, FALSE );
	checkBoxAutoEdit->setChecked( FALSE );
    } else if ( ::qt_cast<QDataBrowser*>(widget) ) {
	setCaption( "Data Browser Wizard" );
	setAppropriate( tablePropertiesPage, FALSE );
	mode = Browser;
	checkBoxAutoEdit->setChecked( TRUE );
    } else if ( ::qt_cast<QDataView*>(widget) ) {
	setCaption( "Data View Wizard" );
	setAppropriate( tablePropertiesPage, FALSE );
	setAppropriate( navigPage, FALSE );
	setAppropriate( sqlPage, FALSE);
	checkCreateFieldLayout->hide();
	checkCreateButtonLayout->hide();
	checkBoxAutoEdit->hide();
	mode = View;
    }

    connect( nextButton(), SIGNAL(clicked()), SLOT(nextPageClicked()) );
    setupPage1();
}


SqlFormWizard::~SqlFormWizard()
{
    appIface->release();
}

void SqlFormWizard::nextPageClicked()
{
    if ( currentPage() == populatePage ) {
	autoPopulate( TRUE );
    }
}

void SqlFormWizard::connectionSelected( const QString &c )
{
    if ( !appIface )
	return;

    DesignerProject *proIface = (DesignerProject*)( (DesignerInterface*)appIface )->currentProject();
    if ( !proIface )
	return;

    listBoxTable->clear();
    QPtrList<DesignerDatabase> databases = proIface->databaseConnections();
    for ( DesignerDatabase *d = databases.first(); d; d = databases.next() ) {
	if ( d->name() == c  || ( d->name() == "(default)" || d->name().isEmpty() ) && c == "(default)")
	    listBoxTable->insertStringList( d->tables() );
    }
    setNextEnabled( databasePage, ( listBoxTable->currentItem() >= 0 ) );
}

void SqlFormWizard::tableSelected( const QString & )
{
    if ( listBoxTable->currentItem() >= 0 ) {
	setNextEnabled( databasePage, TRUE );
    } else {
	setNextEnabled( databasePage, FALSE );
    }

}

void SqlFormWizard::autoPopulate( bool populate )
{
    DesignerProject *proIface = (DesignerProject*)( (DesignerInterface*)appIface )->currentProject();
    if ( !proIface )
	return;
    QPtrList<DesignerDatabase> databases = proIface->databaseConnections();
    listBoxField->clear();
    listBoxSortField->clear();
    listBoxSelectedField->clear();
    if ( populate ) {
	for ( DesignerDatabase *d = databases.first(); d; d = databases.next() ) {
	    if ( d->name() == listBoxConnection->currentText() ||
		 ( ( d->name() == "(default)" || d->name().isEmpty() ) &&
		 listBoxConnection->currentText() == "(default)" ) ) {
		QStringList lst = *d->fields().find( listBoxTable->currentText() );
		// remove primary index fields, if any
		listBoxSortField->insertStringList( lst );
		d->open( FALSE );
#ifndef QT_NO_SQL
		QSqlCursor tab( listBoxTable->currentText(), TRUE, d->connection() );
		QSqlIndex pIdx = tab.primaryIndex();
		for ( uint i = 0; i < pIdx.count(); i++ ) {
		    listBoxField->insertItem( pIdx.field( i )->name() );
		    lst.remove( pIdx.field( i )->name() );
		}
#endif
		d->close();
		listBoxSelectedField->insertStringList( lst );
	    }
	}
    }
}

void SqlFormWizard::fieldDown()
{
    if ( listBoxSelectedField->currentItem() == -1 ||
	 listBoxSelectedField->currentItem() == (int)listBoxSelectedField->count() - 1 ||
	 listBoxSelectedField->count() < 2 )
	return;
    int index = listBoxSelectedField->currentItem() + 1;
    QListBoxItem *i = listBoxSelectedField->item( listBoxSelectedField->currentItem() );
    listBoxSelectedField->takeItem( i );
    listBoxSelectedField->insertItem( i, index );
    listBoxSelectedField->setCurrentItem( i );
}

void SqlFormWizard::fieldUp()
{
    if ( listBoxSelectedField->currentItem() <= 0 ||
	 listBoxSelectedField->count() < 2 )
	return;
    int index = listBoxSelectedField->currentItem() - 1;
    QListBoxItem *i = listBoxSelectedField->item( listBoxSelectedField->currentItem() );
    listBoxSelectedField->takeItem( i );
    listBoxSelectedField->insertItem( i, index );
    listBoxSelectedField->setCurrentItem( i );
}

void SqlFormWizard::removeField()
{
    int i = listBoxSelectedField->currentItem();
    if ( i != -1 ) {
	listBoxField->insertItem( listBoxSelectedField->currentText() );
	listBoxSelectedField->removeItem( i );
    }
}

void SqlFormWizard::addField()
{
    int i = listBoxField->currentItem();
    if ( i == -1 )
	return;
    QString f = listBoxField->currentText();
    if ( !f.isEmpty() )
	listBoxSelectedField->insertItem( f );
    listBoxField->removeItem( i );
}

void SqlFormWizard::addSortField()
{
    int i = listBoxSortField->currentItem();
    if ( i == -1 )
	return;
    QString f = listBoxSortField->currentText();
    if ( !f.isEmpty() )
	listBoxSortedField->insertItem( f + " ASC" );
}

void SqlFormWizard::reSortSortField()
{
    int i = listBoxSortedField->currentItem();
    if ( i != -1 ) {
	QString text = listBoxSortedField->currentText();
	if ( text.mid( text.length() - 3 ) == "ASC" )
	    text = text.mid( 0, text.length() - 3 ) + "DESC";
	else if ( text.mid( text.length() - 4 ) == "DESC" )
	    text = text.mid( 0, text.length() - 4 ) + "ASC";
	listBoxSortedField->removeItem( i );
	listBoxSortedField->insertItem( text, i );
	listBoxSortedField->setCurrentItem( i );
    }
}

void SqlFormWizard::removeSortField()
{
    int i = listBoxSortedField->currentItem();
    if ( i != -1 ) {
	listBoxSortedField->removeItem( i );
    }
}

void SqlFormWizard::sortFieldUp()
{
    if ( listBoxSortedField->currentItem() <= 0 ||
	 listBoxSortedField->count() < 2 )
	return;
    int index = listBoxSortedField->currentItem() - 1;
    QListBoxItem *i = listBoxSortedField->item( listBoxSortedField->currentItem() );
    listBoxSortedField->takeItem( i );
    listBoxSortedField->insertItem( i, index );
    listBoxSortedField->setCurrentItem( i );
}

void SqlFormWizard::sortFieldDown()
{
    if ( listBoxSortedField->currentItem() == -1 ||
	 listBoxSortedField->currentItem() == (int)listBoxSortedField->count() - 1 ||
	 listBoxSortedField->count() < 2 )
	return;
    int index = listBoxSortedField->currentItem() + 1;
    QListBoxItem *i = listBoxSortedField->item( listBoxSortedField->currentItem() );
    listBoxSortedField->takeItem( i );
    listBoxSortedField->insertItem( i, index );
    listBoxSortedField->setCurrentItem( i );
}

void SqlFormWizard::setupDatabaseConnections()
{
    if ( !appIface )
	return;

    DesignerProject *proIface = (DesignerProject*)( (DesignerInterface*)appIface )->currentProject();
    if ( !proIface )
	return;
    proIface->setupDatabases();
    raise();
    setupPage1();
}

void SqlFormWizard::setupPage1()
{
    if ( !appIface )
	return;

    DesignerProject *proIface = (DesignerProject*)( (DesignerInterface*)appIface )->currentProject();
    if ( !proIface )
	return;

    listBoxTable->clear();
    listBoxConnection->clear();
    QPtrList<DesignerDatabase> databases = proIface->databaseConnections();
    QStringList lst;
    for ( DesignerDatabase *d = databases.first(); d; d = databases.next() )
	lst << d->name();
    listBoxConnection->insertStringList( lst );
    if ( lst.count() )
	listBoxConnection->setCurrentItem( 0 );

    setNextEnabled( databasePage, FALSE );
}

static QPushButton *create_widget( QWidget *parent, const char *name,
				   const QString &txt, const QRect &r, DesignerFormWindow *fw )
{
    QPushButton *pb = (QPushButton*)fw->create( "QPushButton", parent, name );
    pb->setText( txt );
    pb->setGeometry( r );
    fw->setPropertyChanged( pb, "text", TRUE );
    fw->setPropertyChanged( pb, "geometry", TRUE );
    return pb;
}

void SqlFormWizard::accept()
{
    if ( !appIface || mode == None )
	return;

#ifndef QT_NO_SQL
    DesignerProject *proIface = (DesignerProject*)( (DesignerInterface*)appIface )->currentProject();
    if ( !widget || !proIface ) {
	SqlFormWizardBase::accept();
	return;
    }

    QString conn = listBoxConnection->currentText();
    QString table = listBoxTable->currentText();
    QStringList lst;
    lst << conn << table;

    if ( !conn.isEmpty() && !table.isEmpty() ) {
	formWindow->setProperty( widget, "database", lst );
	formWindow->setPropertyChanged( widget, "database", TRUE );
    }

    if ( !editFilter->text().isEmpty() ) {
	widget->setProperty( "filter", editFilter->text() );
	formWindow->setPropertyChanged( widget, "filter", TRUE );
    }

    if ( listBoxSortedField->count() ) {
	QStringList lst;
	for ( uint i = 0; i < listBoxSortedField->count(); ++i )
	    lst << listBoxSortedField->text( i );
	widget->setProperty( "sort", lst );
	formWindow->setPropertyChanged( widget, "sort", TRUE );
    }

    QPtrList<DesignerDatabase> databases = proIface->databaseConnections();
    DesignerDatabase *database = 0;
    for ( DesignerDatabase *d = databases.first(); d; d = databases.next() ) {
	if ( d->name() == listBoxConnection->currentText() || ( d->name() == "(default)" || d->name().isEmpty() ) && listBoxConnection->currentText() == "(default)" ) {
	    database = d;
	    d->open( FALSE );
	    break;
	}
    }

    if (!database) {
	return;
    }
    QSqlCursor tab( listBoxTable->currentText(), TRUE, database->connection() );
    int columns = 2;

    QSqlEditorFactory * f = QSqlEditorFactory::defaultFactory();

    QWidget * editorDummy;
    QWidget * editor;
    QLabel * label;

    int visibleFields = listBoxSelectedField->count();
    int numPerColumn = visibleFields / columns;
    if( (visibleFields % columns) > 0)
	numPerColumn++;

    int row = 0;
    const int SPACING = 25;
    const int COL_SPACING = SPACING*9;

    uint j;
    switch ( mode ) {
    case None:
	break;
    case View:
    case Browser: {

	if ( mode == Browser && !checkBoxAutoEdit->isChecked() ) {
	    ((QDataBrowser*)widget)->setAutoEdit( FALSE );
	    formWindow->setPropertyChanged( widget, "autoEdit", TRUE );
	}

	formWindow->clearSelection();
	bool createFieldLayout = checkCreateFieldLayout->isChecked();
	bool createButtonLayout = checkCreateButtonLayout->isChecked();
	bool createLayouts = checkCreateLayouts->isChecked();
	bool labelAbove = radioLabelsTop->isOn();
	uint numCols = spinNumberOfColumns->text().toInt();
	uint currentCol = 0;
	uint fieldsPerCol = listBoxSelectedField->count();
	uint fieldsInCol = 0;
	if ( listBoxSelectedField->count() )
	    fieldsPerCol = listBoxSelectedField->count() / numCols;
	/* labels and data field editors */
	for( j = 0; j < listBoxSelectedField->count(); j++ ){

	    QSqlField* field = tab.field( listBoxSelectedField->text( j ) );
	    if ( !field )
		continue;

	    /* label */
	    QString labelName = field->name();
	    labelName = labelName.mid(0,1).upper() + labelName.mid(1);
	    label = (QLabel*)formWindow->create( "QLabel", widget,
						 QString( "label" + labelName ) );
	    label->setText( labelName );
	    label->setGeometry( SPACING + currentCol*COL_SPACING, row+SPACING,
				SPACING*3, SPACING );

	    formWindow->setPropertyChanged( label, "geometry", TRUE );
	    formWindow->setPropertyChanged( label, "text", TRUE );

	    /* editor */
	    editorDummy = f->createEditor( widget, field );
	    editor = formWindow->create( editorDummy->className(), widget,
					 QString( QString( editorDummy->className() )
						  + labelName) );
	    delete editorDummy;
	    if ( labelAbove ) {
		row += SPACING;
		editor->setGeometry(SPACING + currentCol*COL_SPACING, row+SPACING,
				    SPACING*3, SPACING );
	    } else {
		editor->setGeometry(SPACING * 5 + currentCol*COL_SPACING, row+SPACING,
				    SPACING*3, SPACING );
	    }
	    formWindow->setPropertyChanged( editor, "geometry", TRUE );
	    if ( QString(editor->className()) == "QLineEdit" &&
		 (field->type() == QVariant::Double ||
		  field->type() == QVariant::Int ||
		  field->type() == QVariant::UInt ) ) {
		/* default right-align numerics */
		//##
		((QLineEdit*)editor)->setAlignment( Qt::AlignRight );
		formWindow->setPropertyChanged( editor, "alignment", TRUE );
	    }
	    if ( ::qt_cast<QSpinBox*>(editor) ) {
		( (QSpinBox*)editor )->setMaxValue( INT_MAX );
		formWindow->setPropertyChanged( editor, "maxValue", TRUE );
	    }
	    QStringList lst;
	    lst << conn << table << field->name();
	    formWindow->setProperty( editor, "database", lst );
	    formWindow->setPropertyChanged( editor, "database", TRUE );

	    /* geometry */
	    if ( createFieldLayout ) {
		formWindow->selectWidget( label );
		formWindow->selectWidget( editor );
	    }

	    row += SPACING + 5;
	    fieldsInCol++;
	    if ( ( fieldsInCol >= fieldsPerCol ) && ( currentCol < numCols-1 ) ) {
		currentCol++;
		fieldsInCol = 0;
		row = 0;
	    }
	}

	if ( listBoxSelectedField->count() ) {
	    if ( createFieldLayout )
		formWindow->layoutG();
	    row += SPACING;
	}

	if ( mode == Browser ) {
	    if ( checkBoxNavig->isChecked() ) {
		formWindow->clearSelection();
		currentCol = 0;
		if ( checkBoxFirst->isChecked() ) {
		    QPushButton *pb = create_widget( widget, "PushButtonFirst",
						     "|< &First",
						     QRect( 3 * SPACING * currentCol, row+SPACING, SPACING * 3, SPACING ),
						     formWindow );
		    formWindow->addConnection( pb, "clicked()", widget, "first()" );
		    formWindow->addConnection( widget, "firstRecordAvailable( bool )",
					       pb, "setEnabled( bool )" );
		    currentCol++;
		    formWindow->selectWidget( pb );
		}
		if ( checkBoxPrev->isChecked() ) {
		    QPushButton *pb = create_widget( widget, "PushButtonPrev",
						     "<< &Prev",
						     QRect( 3 * SPACING * currentCol, row+SPACING, SPACING * 3, SPACING ),
						     formWindow );
		    formWindow->addConnection( pb, "clicked()", widget, "prev()" );
		    formWindow->addConnection( widget, "prevRecordAvailable( bool )",
					       pb, "setEnabled( bool )" );
		    currentCol++;
		    formWindow->selectWidget( pb );
		}
		if ( checkBoxNext->isChecked() ) {
		    QPushButton *pb = create_widget( widget, "PushButtonNext",
						     "&Next >>",
						     QRect( 3 * SPACING * currentCol, row+SPACING, SPACING * 3, SPACING ),
						     formWindow );
		    formWindow->addConnection( pb, "clicked()", widget, "next()" );
		    formWindow->addConnection( widget, "nextRecordAvailable( bool )", pb,
					       "setEnabled( bool )" );
		    currentCol++;
		    formWindow->selectWidget( pb );
		}
		if ( checkBoxLast->isChecked() ) {
		    QPushButton *pb = create_widget( widget, "PushButtonLast", "&Last >|",
						     QRect( 3 * SPACING * currentCol, row+SPACING, SPACING*3, SPACING ), formWindow );
		    formWindow->addConnection( pb, "clicked()", widget, "last()" );
		    formWindow->addConnection( widget, "lastRecordAvailable( bool )", pb,
					       "setEnabled( bool )" );
		    currentCol++;
		    formWindow->selectWidget( pb );
		}
		if ( createButtonLayout )
		    formWindow->layoutH();
	    }
	    if ( checkBoxEdit->isChecked() ) {
		formWindow->clearSelection();
		row += SPACING;
		currentCol = 0;
		if ( checkBoxInsert->isChecked() ) {
		    QPushButton *pb = create_widget( widget, "PushButtonInsert", "&Insert",
						     QRect( 3 * SPACING * currentCol, row+SPACING, SPACING * 3, SPACING ), formWindow );
		    formWindow->addConnection( pb, "clicked()", widget, "insert()" );
		    currentCol++;
		    formWindow->selectWidget( pb );
		}
		if ( checkBoxUpdate->isChecked() ) {
		    QPushButton *pb = create_widget( widget, "PushButtonUpdate", "&Update",
						     QRect( 3 * SPACING * currentCol, row+SPACING, SPACING * 3, SPACING ), formWindow );
		    formWindow->addConnection( pb, "clicked()", widget, "update()" );
		    currentCol++;
		    formWindow->selectWidget( pb );
		}
		if ( checkBoxDelete->isChecked() ) {
		    QPushButton *pb = create_widget( widget, "PushButtonDelete", "&Delete",
						     QRect( 3 * SPACING * currentCol, row+SPACING, SPACING * 3, SPACING ), formWindow );
		    formWindow->addConnection( pb, "clicked()", widget, "del()" );
		    currentCol++;
		    formWindow->selectWidget( pb );
		}
		if ( createButtonLayout )
		    formWindow->layoutH();
	    }
	}
	if ( createLayouts )
	    formWindow->layoutGContainer( widget );
	formWindow->clearSelection();
	break;
    }
    case Table:
	{
	    QDataTable* sqlTable = ((QDataTable*)widget);
	    if ( checkBoxAutoEdit->isChecked() ) {
		sqlTable->setAutoEdit( TRUE );
		formWindow->setPropertyChanged( sqlTable, "autoEdit", TRUE );
	    }

	    if ( checkBoxReadOnly->isChecked() ) {
		sqlTable->setReadOnly( TRUE );
		formWindow->setPropertyChanged( sqlTable, "readOnly", TRUE );
	    } else {
		if ( checkBoxConfirmInserts->isChecked() ) {
		    sqlTable->setConfirmInsert( TRUE );
		    formWindow->setPropertyChanged( sqlTable, "confirmInsert", TRUE );
		}
		if ( checkBoxConfirmUpdates->isChecked() ) {
		    sqlTable->setConfirmUpdate( TRUE );
		    formWindow->setPropertyChanged( sqlTable, "confirmUpdate", TRUE );
		}
		if ( checkBoxConfirmDeletes->isChecked() ) {
		    sqlTable->setConfirmDelete( TRUE );
		    formWindow->setPropertyChanged( sqlTable, "confirmDelete", TRUE );
		}
		if ( checkBoxConfirmCancels->isChecked() ) {
		    sqlTable->setConfirmCancels( TRUE );
		    formWindow->setPropertyChanged( sqlTable, "confirmCancels", TRUE );
		}
	    }
	    if ( checkBoxSorting->isChecked() ) {
		sqlTable->setSorting( TRUE );
		formWindow->setPropertyChanged( sqlTable, "sorting", TRUE );
	    }

	    QMap<QString, QString> columnFields;
	    sqlTable->setNumCols( listBoxSelectedField->count() ); // no need to change property through mdbIface here, since QDataTable doesn't offer that through Designer
	    for( j = 0; j < listBoxSelectedField->count(); j++ ){

		QSqlField* field = tab.field( listBoxSelectedField->text( j ) );
		if ( !field )
		    continue;

		QString labelName = field->name();
		labelName = labelName.mid(0,1).upper() + labelName.mid(1);

		((QTable*)widget)->horizontalHeader()->setLabel( j, labelName );

		columnFields.insert( labelName, field->name() );
	    }
	    formWindow->setColumnFields( widget, columnFields );
	    break;
	}
    }

    database->close();
#endif

    SqlFormWizardBase::accept();
}
