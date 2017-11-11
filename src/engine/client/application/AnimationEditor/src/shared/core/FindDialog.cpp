// ======================================================================
//
// FindDialog.cpp
// copyright(c) 2006 Sony Online Entertainment
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"

#include <qlineedit.h>
#include <qmessagebox.h>

#include "FindDialog.h"
#include "FindDialog.moc"

#include "MainWindow.h"


FindDialog::FindDialog(QWidget* theParent, const char* theName)
: BaseFindDialog(theParent, theName)
{
	
}

//----------------------------------------------------------------------

FindDialog::~FindDialog()
{
	
}

//----------------------------------------------------------------------

void FindDialog::onButtonFindNext()
{
	if(lineEditFind->text().length() == 0)
	{
		QMessageBox::warning(this, "find error", "Find field empty", QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}
	
	MainWindow *mainWindow = dynamic_cast<MainWindow* >(parentWidget());
	if(mainWindow)
	{
		mainWindow->selectMatchingItem(lineEditFind->text().ascii(),false);
	}
	else
	{
		QMessageBox::warning(this, "find error", "Unable to get ahold of parent widget", QMessageBox::Ok, QMessageBox::NoButton);
	}
}

