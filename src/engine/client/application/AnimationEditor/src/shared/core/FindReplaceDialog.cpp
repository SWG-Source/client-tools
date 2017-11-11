// ======================================================================
//
// FindReplaceDialog.cpp
// copyright(c) 2006 Sony Online Entertainment
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "FindReplaceDialog.h"
#include "FindReplaceDialog.moc"

#include <QLineEdit.h>
#include <QMessageBox.h>

#include "MainWindow.h"


FindReplaceDialog::FindReplaceDialog(QWidget* theParent, const char* theName)
: BaseFindReplaceDialog(theParent, theName)
{
}

//----------------------------------------------------------------------

FindReplaceDialog::~FindReplaceDialog()
{
}

//----------------------------------------------------------------------

void FindReplaceDialog::onButtonFindNext()
{
	if(lineEditFind->text().length() == 0)
	{
		QMessageBox::warning(this, "find error", "Find field empty", QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}
	
	MainWindow *mainWindow = dynamic_cast<MainWindow* >(parentWidget());
	if(mainWindow)
	{
		mainWindow->selectMatchingItem(lineEditFind->text().ascii(),true);
	}
	else
	{
		QMessageBox::warning(this, "find error", "Unable to get ahold of parent widget", QMessageBox::Ok, QMessageBox::NoButton);
	}

}

//----------------------------------------------------------------------


void FindReplaceDialog::onButtonReplace()
{
	if(lineEditReplace->text().length() == 0)
	{
		QMessageBox::warning(this, "replace error", "Replace field empty", QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}

	MainWindow *mainWindow = dynamic_cast<MainWindow* >(parentWidget());
	if(mainWindow)
	{
		mainWindow->replaceSelectedText(lineEditFind->text().ascii(),lineEditReplace->text().ascii());
	}
	else
	{
		QMessageBox::warning(this, "replace error", "Unable to get ahold of parent widget", QMessageBox::Ok, QMessageBox::NoButton);
	}
}

//----------------------------------------------------------------------


void FindReplaceDialog::onButtonReplaceNext()
{
	onButtonReplace();
	onButtonFindNext();
}

//----------------------------------------------------------------------


void FindReplaceDialog::onButtonReplaceAll()
{
	if(lineEditFind->text().length() == 0)
	{
		QMessageBox::warning(this, "replace all error", "Find field empty", QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}

	if(lineEditReplace->text().length() == 0)
	{
		QMessageBox::warning(this, "replace all error", "Replace field empty", QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}


	MainWindow *mainWindow = dynamic_cast<MainWindow* >(parentWidget());
	if(mainWindow)
	{
		mainWindow->replaceAllText(lineEditFind->text().ascii(),lineEditReplace->text().ascii());
	}
	else
	{
		QMessageBox::warning(this, "replace all error", "Unable to get ahold of parent widget", QMessageBox::Ok, QMessageBox::NoButton);
	}

	
}

//----------------------------------------------------------------------