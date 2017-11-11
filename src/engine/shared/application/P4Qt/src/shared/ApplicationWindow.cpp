// ======================================================================
//
// ApplicationWindow.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

//include other headers from this project that we need
#include "ApplicationWindow.h"
#include "ApplicationWindow.moc"

#include "DepotListViewItem.h"
#include "DepotsClientUser.h"
#include "Dialog.h"
#include "OpenedClientUser.h"
#include "Pixmaps.h"

#include <qapplication.h>
#include <qcursor.h>
#include <qdatetime.h>
#include <qlistview.h>
#include <qmenubar.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <qtextedit.h>

#include <qhbox.h>
#include <qvbox.h>
#include <qpushbutton.h>

// ======================================================================
/**
 * Default constructor.
 */

ApplicationWindow::ApplicationWindow(QApplication &application)
: QMainWindow( 0, "application main window", WDestructiveClose ),
	m_application(application),
	m_depot(NULL),
	m_changelists(NULL),
	m_output(NULL),
	m_summary(false)
{
	// set the application icon to a "Qt" symbol, replace with verant icon later if possible
	setIcon(*Pixmaps::applicationIcon);

	// create the file menu
	QPopupMenu *fileMenu = new QPopupMenu(this);
	menuBar()->insertItem( "&File", fileMenu );
	fileMenu->insertItem( "&Clear window", this, SLOT(clear()), ALT+Key_C);
	fileMenu->insertItem( "&Opened", this, SLOT(opened()), ALT+Key_O);
	fileMenu->insertItem( "E&xit", qApp, SLOT(closeAllWindows()), ALT+Key_F4);

	// create the main horizontal splitter
	QSplitter *mainSplitter = new QSplitter(this);
	mainSplitter->setOrientation(Qt::Vertical);

	// split the top half in two
	QSplitter *topSplitter = new QSplitter(mainSplitter);
	topSplitter->setOrientation(Qt::Horizontal);
	
	// put the depot in the top left pane
	m_depot = new QListView(topSplitter);
	m_depot->setRootIsDecorated(true);
	m_depot->addColumn("Depot");
	m_depot->setSelectionMode(QListView::Extended);

	// put the changelists in the top right pane
	m_changelists = new QListView(topSplitter);
	m_changelists->setRootIsDecorated(true);
	m_changelists->addColumn("Changelists");
  QObject::connect(m_changelists, SIGNAL(contextMenuRequested(QListViewItem *, const QPoint &, int)), this, SLOT(changelistContextMenu(QListViewItem *, const QPoint &, int)));

	// put the output window in the bottom pane
	m_output = new QTextEdit(mainSplitter);
	m_output->setReadOnly(true);
	m_output->setWordWrap(QTextEdit::NoWrap);
	m_output->setVScrollBarMode(QScrollView::AlwaysOn);
	m_output->setHScrollBarMode(QScrollView::AlwaysOn);
	outputMessage(MT_status, "Output pane");
  QObject::connect(m_depot, SIGNAL(contextMenuRequested(QListViewItem *, const QPoint &, int)), this, SLOT(depotContextMenu(QListViewItem *, const QPoint &, int)));

	setCentralWidget(mainSplitter);
	statusBar()->show();
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

void ApplicationWindow::p4(bool tags, const std::vector<const char *> &args, ClientUser &clientUser)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	ClientApi client;
	Error e;

	if (tags)
		client.SetProtocol("tag", "");
	client.Init( &e );

	const int argc = static_cast<int>(args.size());
	const char * const * const argv = &args[0];

	// output the p4 command to the status window
	QString command("Executing: p4 ");
	for (int i = 0; i < argc-1; ++i)
		command += argv[i] +QString(" ");
	command += argv[argc-1] + QString("\n");
	outputMessage(MT_status, command);
	
	client.SetArgv(argc-1, const_cast<char * const *>(argv+1));
	client.Run(argv[0], &clientUser);
	client.Final(&e);

	if (m_errors.empty())
		outputMessage(MT_status, "Completed.  No errors reported\n");
	else
	{
		m_summary = true;	
	
			outputMessage(MT_error, "Completed.  Error summary:\n");

			int count = 0;
			ErrorList::const_iterator iEnd = m_errors.end();
			for (ErrorList::const_iterator i = m_errors.begin(); i != iEnd; ++i, ++count)
					outputMessage(MT_error, *i);

			if (count == 1)
				outputMessage(MT_error, QString("1 error reported\n"));
			else
				outputMessage(MT_error, QString("%1 errors reported\n").arg(count));

		m_summary = false;	
	}

	QApplication::restoreOverrideCursor();
}

// ======================================================================
// depot window functions
// ======================================================================

void ApplicationWindow::addDepot(QString depotName)
{
	DepotListViewItem *newDepot = new DepotListViewItem(*this, false, m_depot, QString("//") + depotName);
	newDepot->setPixmap(0, *Pixmaps::depotIcon);
}

// ----------------------------------------------------------------------

void ApplicationWindow::getSelectedDepotFiles(QListViewItem *item, std::vector<const char *> &files)
{
	// recursive search for selected files
	for ( ; item; item = item->nextSibling())
	{
		if (item->isSelected())
			files.push_back(dynamic_cast<DepotListViewItem*>(item)->getSyncName());
		getSelectedDepotFiles(item->firstChild(), files);
	}
}

// ----------------------------------------------------------------------

void ApplicationWindow::getSelectedDepotFiles(std::vector<const char *> &files)
{
	// search the whole tree for selected items
	getSelectedDepotFiles(m_depot->firstChild(), files);
}

// ----------------------------------------------------------------------

void ApplicationWindow::depots()
{
	m_depot->clearSelection();
	m_depot->clear();

	std::vector<const char *> argv;
	argv.push_back("depots");
	DepotsClientUser clientUser(m_application, *this);
	p4(false, argv, clientUser);	
}

// ----------------------------------------------------------------------

void ApplicationWindow::previewSync()
{
	std::vector<const char *> argv;
	argv.push_back("sync");
	argv.push_back("-n");
	getSelectedDepotFiles(argv);

	OpenedClientUser clientUser(m_application, *this);
	p4(true, argv, clientUser);	
}

// ----------------------------------------------------------------------

void ApplicationWindow::sync()
{
	std::vector<const char *> argv;
	argv.push_back("sync");
	getSelectedDepotFiles(argv);
	OpenedClientUser clientUser(m_application, *this);
	p4(true, argv, clientUser);	
}

// ----------------------------------------------------------------------

void ApplicationWindow::edit()
{
	std::vector<const char *> argv;
	argv.push_back("edit");
	getSelectedDepotFiles(argv);
	OpenedClientUser clientUser(m_application, *this);
	p4(true, argv, clientUser);	

	// rebuild the opened list
	opened();
}

// ----------------------------------------------------------------------

void ApplicationWindow::depotContextMenu(QListViewItem *item, const QPoint &point, int /* column */)
{
	if (item)
	{
		QPopupMenu popupMenu(this);
		popupMenu.insertItem("&Sync to head revision", this, SLOT(sync()));
		popupMenu.insertItem("&Preview sync to head revision", this, SLOT(previewSync()));
		popupMenu.insertItem("Open for &edit", this, SLOT(edit()));
		popupMenu.exec(point);
	}
}

// ======================================================================
// changelist window functions
// ======================================================================

void ApplicationWindow::addOpenedFile(QString changelist, QString file, bool binary)
{
	const char *ascii_changelist = changelist.latin1();
	const char *ascii_file  = file.latin1();

	ascii_changelist;
	ascii_file;

	QListViewItem *parent = m_changelists->findItem(changelist, 0);
	if (!parent)
	{
		parent = new QListViewItem(m_changelists, changelist);
		parent->setPixmap(0, *Pixmaps::changeListIcon);
	}

	QListViewItem *item = new QListViewItem(parent, file);
	if (binary)
		item->setPixmap(0, *Pixmaps::binaryFileIcon);
	else
		item->setPixmap(0, *Pixmaps::textFileIcon);
}

// ----------------------------------------------------------------------

void ApplicationWindow::getSelectedChangelistsFiles(QListViewItem *item, std::vector<const char *> &files)
{
	// recursive search for selected files
	for ( ; item; item = item->nextSibling())
	{
		if (item->isSelected())
			files.push_back(item->text(0));
		getSelectedChangelistsFiles(item->firstChild(), files);
	}
}

// ----------------------------------------------------------------------

void ApplicationWindow::getSelectedChangelistsFiles(std::vector<const char *> &files)
{
	// search the whole tree for selected items
	getSelectedChangelistsFiles(m_changelists->firstChild(), files);
}

// ----------------------------------------------------------------------

void ApplicationWindow::opened()
{
	m_changelists->clearSelection();
	m_changelists->clear();

	std::vector<const char *> argv;
	argv.push_back("opened");
	OpenedClientUser clientUser(m_application, *this);
	p4(true, argv, clientUser);	
}

// ----------------------------------------------------------------------

void ApplicationWindow::revert()
{
	std::vector<const char *> argv;
	argv.push_back("revert");
	getSelectedChangelistsFiles(argv);
	OpenedClientUser clientUser(m_application, *this);
	p4(true, argv, clientUser);	

	// rebuild the opened list
	opened();
}

// ----------------------------------------------------------------------

void ApplicationWindow::submit()
{
	TextEditDialog dialog(this, "Submit");

	if (dialog.exec() == QDialog::Accepted)
	{
		// rebuild the opened list
		opened();
	}
}

// ----------------------------------------------------------------------

void ApplicationWindow::changelistContextMenu(QListViewItem *item, const QPoint &point, int /* column */)
{
	if (item)
	{
		if (item->depth() == 0)
		{
			QPopupMenu popupMenu(this);
			popupMenu.insertItem("&Submit", this, SLOT(submit()));
			popupMenu.exec(point);
		}
		else
		{
			QPopupMenu popupMenu(this);
			popupMenu.insertItem("&Revert", this, SLOT(revert()));
			popupMenu.exec(point);
		}
	}
}

// ======================================================================
// output window functions
// ======================================================================

void ApplicationWindow::outputMessage(MessageType type, QString message)
{
	switch (type)
	{
		case MT_normal:
			m_output->setColor(QColor(0, 0, 0));
			break;

		case MT_error:
			m_output->setColor(QColor(255, 0, 0));
			if (!m_summary)
				m_errors.push_back(message);
			break;

		case MT_status:
			m_output->setColor(QColor(0, 0, 255));
			break;

		default:
			m_output->setColor(QColor(255, 0, 255));
			break;
	}

	int contentsX = m_output->contentsX();
	m_output->append(QTime::currentTime().toString("hh:mm:ss.zzz") + QString(": ") + 	message);
	m_output->setContentsPos(contentsX, m_output->contentsY());
}

// ----------------------------------------------------------------------

void ApplicationWindow::clear()
{
	m_output->clear();
	statusBar()->clear();
}

// ======================================================================
