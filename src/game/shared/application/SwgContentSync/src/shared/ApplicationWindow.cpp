// ======================================================================
//
// ApplicationWindow.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstSwgContentSync.h"
#include "ApplicationWindow.h"
#include "ApplicationWindow.moc"

#include "ContentSyncClientUser.h"
#include "TextEdit.h"

#include <qapplication.h>
#include <qdatetime.h>
#include <qmenubar.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qstatusbar.h>

#include "res\application_icon.xpm"

// ======================================================================

static char *argv[] =
{
	"-n",
	"//depot/swg/current/data/...@swg-content-label",
	"//depot/swg/current/doc/...",
	"//depot/swg/current/dsrc/...@swg-content-label",
	"//depot/swg/current/exe/...@swg-content-label",
	"//depot/swg/current/src/game/server/database/...@swg-content-label",
	"//depot/swg/current/plugin/..."
};
static const int argc = sizeof(argv) / sizeof(argv[0]);

// ======================================================================

ApplicationWindow::ApplicationWindow(QApplication &application)
: QMainWindow( 0, "application main window", WDestructiveClose ),
	m_application(application),
	m_textEdit(NULL),
	m_fileMenu(NULL),
	m_forceResyncItemNumber(-1),
	m_summary(false),
	m_client(NULL)
{
	// set the application icon to a "Qt" symbol, replace with verant icon later if possible
	QPixmap appIcon(application_icon);
	setIcon(appIcon);

	// create the file menu
	m_fileMenu = new QPopupMenu(this);
	menuBar()->insertItem( "&File", m_fileMenu );
	m_fileMenu->insertItem( "&Sync", this, SLOT(sync()), ALT+Key_S);
	m_fileMenu->insertItem( "&Preview sync", this, SLOT(previewSync()), ALT+Key_P);
	m_fileMenu->insertItem( "&Clear window", this, SLOT(clear()), ALT+Key_C);
	m_forceResyncItemNumber = m_fileMenu->insertItem( "&Force resync error files", this, SLOT(forceResyncErrorFiles()), ALT+Key_F);
	m_fileMenu->insertItem( "E&xit", qApp, SLOT(closeAllWindows()), ALT+Key_F4);
	m_fileMenu->setItemEnabled(m_forceResyncItemNumber, false);

	// setup the text display window
	m_textEdit = new TextEdit(this);
	m_textEdit->setReadOnly(true);
	m_textEdit->setWordWrap(QTextEdit::NoWrap);
	m_textEdit->setVScrollBarMode(QScrollView::AlwaysOn);
	m_textEdit->setHScrollBarMode(QScrollView::AlwaysOn);
	setCentralWidget(m_textEdit);

	statusBar()->show();

	if (application.argc() > 1)
		m_client = application.argv()[1];
}

// ----------------------------------------------------------------------
/**
 * Destructor. Cleans up the allocated memory (though Qt should delete all children objects
 * of this object.  Delete non-child objects like the printer.
 */
ApplicationWindow::~ApplicationWindow()
{
}

// ----------------------------------------------------------------------
/**
 * Function called when the app tries to exit.  We could have a dialog
 * ask about saving, etc, but it isn't necessary, so we trivially accept it.
 */

void ApplicationWindow::closeEvent( QCloseEvent* ce )
{
	ce->accept();
}

// ----------------------------------------------------------------------

void ApplicationWindow::appendMessage(MessageType type, QString message)
{
	switch (type)
	{
		case MT_normal:
			m_textEdit->setColor(QColor(0, 0, 0));
			break;

		case MT_error:
			m_textEdit->setColor(QColor(255, 0, 0));
			break;

		case MT_status:
			m_textEdit->setColor(QColor(0, 0, 255));
			break;

		default:
			break;
	}

	m_textEdit->setAppending(true);
	m_textEdit->append(QTime::currentTime().toString("hh:mm:ss.zzz") + QString(": ") + 	message);
	m_textEdit->setAppending(false);

	if (!m_summary && type == MT_error)
	{
		const char *prefix = "Can't clobber writable file ";
		if (message.startsWith(prefix))
		{
			m_errorFiles.push_back(message.mid(strlen(prefix)).stripWhiteSpace());
			m_fileMenu->setItemEnabled(m_forceResyncItemNumber, true);
		}
	}

	m_application.processEvents();
}

// ----------------------------------------------------------------------
/**
 * Sync the data.
 */

void ApplicationWindow::sync(bool preview, const QString &syncType)
{
	statusBar()->message(syncType + QString(" in progess..." ));
	appendMessage(MT_status, syncType + QString(" started\n"));

	ClientApi client;
	Error e;
	client.Init( &e );

	ContentSyncClientUser clientUser(*this);

	if (preview)
		client.SetArgv(argc, argv);
	else
		client.SetArgv(argc-1, argv+1);

	if (m_client)
		client.SetClient(m_client);

	client.Run("sync", &clientUser);
	client.Final (&e);

	appendMessage(MT_status, syncType + QString(" done\n"));

	{
		ContentSyncClientUser::ErrorList::const_iterator i = clientUser.errorsBegin();
		ContentSyncClientUser::ErrorList::const_iterator iEnd = clientUser.errorsEnd();
		if (i != iEnd)
		{
			m_summary = true;
			appendMessage(MT_error, "Error summary:\n");
			int count = 0;
			for ( ; i != iEnd; ++i, ++count)
				appendMessage(MT_error, *i);

			if (count == 1)
				appendMessage(MT_error, QString("1 error reported\n"));
			else
				appendMessage(MT_error, QString("%1 errors reported\n").arg(count));

			m_summary = false;
		}
		else
			appendMessage(MT_status, "No errors reported\n");
	}

	statusBar()->message(syncType + QString(" done."));
}

// ----------------------------------------------------------------------

void ApplicationWindow::forceResyncErrorFiles()
{
	statusBar()->message("Force resync error files in progess..." );
	appendMessage(MT_status, "Force resync error files in progess...");

	ClientApi client;
	Error e;
	client.Init( &e );

	ContentSyncClientUser clientUser(*this);

	std::vector<const char*> files;
	files.reserve(m_errorFiles.size() + 1);

	files.push_back("-f");
	for (uint i = 0; i < m_errorFiles.size(); ++i)
		files.push_back(m_errorFiles[i]);

	client.SetArgv(files.size(), const_cast<char * const *>(&files[0]));
	client.Run("sync", &clientUser);
	client.Final (&e);

	appendMessage(MT_status, "Force resync error files done.");

	{
		ContentSyncClientUser::ErrorList::const_iterator i = clientUser.errorsBegin();
		ContentSyncClientUser::ErrorList::const_iterator iEnd = clientUser.errorsEnd();
		if (i != iEnd)
		{
			appendMessage(MT_error, "Error summary:\n");
			int count = 0;
			for ( ; i != iEnd; ++i, ++count)
				appendMessage(MT_error, *i);

			if (count == 1)
				appendMessage(MT_error, QString("1 error reported\n"));
			else
				appendMessage(MT_error, QString("%1 errors reported\n").arg(count));
		}
		else
			appendMessage(MT_status, "No errors reported\n");
	}

	statusBar()->message("Force resync error files done.");

	m_errorFiles.clear();	
	m_fileMenu->setItemEnabled(m_forceResyncItemNumber, false);
}

// ----------------------------------------------------------------------

void ApplicationWindow::clear()
{
	m_errorFiles.clear();	
	m_fileMenu->setItemEnabled(m_forceResyncItemNumber, false);
	m_textEdit->clear();
	statusBar()->clear();
}

// ----------------------------------------------------------------------

void ApplicationWindow::sync()
{
	m_errorFiles.clear();	
	m_fileMenu->setItemEnabled(m_forceResyncItemNumber, false);
	sync(false, "Sync");
}

// ----------------------------------------------------------------------

void ApplicationWindow::previewSync()
{
	m_errorFiles.clear();	
	m_fileMenu->setItemEnabled(m_forceResyncItemNumber, false);
	sync(true, "Preview sync");
}

// ----------------------------------------------------------------------

enum MemoryManagerNotALeak
{
	MM_notALeak
};

void *operator new(size_t size, MemoryManagerNotALeak)
{
	return operator new(size);
}

// ----------------------------------------------------------------------
