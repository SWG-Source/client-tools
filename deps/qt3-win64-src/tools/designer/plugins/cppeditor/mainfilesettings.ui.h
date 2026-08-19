/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <designerinterface.h>

void CppMainFile::init()
{
}

void CppMainFile::setup( QUnknownInterface *appIface )
{
    DesignerInterface *dIface = 0;
    appIface->queryInterface( IID_Designer, (QUnknownInterface**)&dIface );
    if ( !dIface )
	return;
    QStringList l = dIface->currentProject()->formNames();
    editFileName->setText( "main.cpp" );
    listForms->clear();
    listForms->insertStringList( l );
    listForms->setCurrentItem( 0 );
    updateOkButton();
    editFileName->setFocus();
    editFileName->selectAll();
}

void CppMainFile::updateOkButton()
{
    buttonOk->setEnabled( editFileName->text().length() > 0 && listForms->currentItem() != -1 );
}


