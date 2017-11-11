// ======================================================================
//
// Main.cpp
// Copyright 2004, Sony Online Entertainment Inc.
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "MainWindow.h"
#include "QuestEditorConfig.h"
#include "QuestEditorConstants.h"

// ----------------------------------------------------------------------

#include <qapplication.h>
#include <qimage.h>

// ----------------------------------------------------------------------

#include "images/questeditor_small.xpm"

// ----------------------------------------------------------------------

int main(int argc, char ** argv)
{
	QApplication a(argc, argv);

	//-- install and valid configuration file
	if (!QuestEditorConfig::install())
	{
		QuestEditorConfig::remove();
		return -1;
	}

	MainWindow * mw = new MainWindow();
	a.setMainWidget(mw);
	mw->setCaption(QUEST_EDITOR_VERSION " (Built " __DATE__ " - " __TIME__ ")");
	mw->setIcon(QPixmap(questeditor_small_xpm));

	QRect const screen = QApplication::desktop()->screenGeometry();

	int const x = (screen.width() - mw->frameGeometry().width()) / 2;
	int const y = (screen.height() - mw->frameGeometry().height()) / 2;

	mw->move(x, y);

	mw->show();
	IGNORE_RETURN(QApplication::connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit())));
	int res = a.exec();

	QuestEditorConfig::remove();
	return res; //lint !e429
}

// ======================================================================
