/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions use Qt Designer which will
** update this file, preserving your code. Create an init() function in place
** of a constructor, and a destroy() function in place of a destructor.
*****************************************************************************/
#include "dialogform.h"
#include "extension.h"
#include <qapplication.h>
#include <qcheckbox.h>
#include <qlineedit.h>

void MainForm::init()
{
    sessions = FALSE;
    logging = FALSE;
    log_filename = QString::null;
    log_errors = TRUE;
    log_actions = TRUE;
}

void MainForm::optionsDlg()
{
    DialogForm *dlg = new DialogForm( this, "dialog", TRUE );
    Extension *ext = (Extension*)dlg->extension()->qt_cast( "Extension" );
    if ( !ext )
	return;
    dlg->sessionsCheckBox->setChecked( sessions );
    dlg->loggingCheckBox->setChecked( logging );
    ext->logfileLineEdit->setText( log_filename );
    ext->logErrorsCheckBox->setChecked( log_errors );

    if ( dlg->exec() ) {
	sessions = dlg->sessionsCheckBox->isChecked();
	logging = dlg->loggingCheckBox->isChecked();
	log_filename = ext->logfileLineEdit->text();
	log_errors = ext->logErrorsCheckBox->isChecked();
    }
}


void MainForm::quit()
{
    QApplication::exit( 0 );
}
