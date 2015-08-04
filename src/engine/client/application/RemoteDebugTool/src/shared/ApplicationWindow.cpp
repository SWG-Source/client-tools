// ======================================================================
//
// ApplicationWindow.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

//include other headers from this project that we need
#include "FirstRemoteDebugTool.h"
#include "ApplicationWindow.h"
#include "ApplicationWindow.moc"

#include "Archive/ByteStream.h"
#include "sharedNetwork/Service.h"
#include "sharedDebug/RemoteDebug_inner.h"

#include "ChannelWindow.h"
#include "RemoteDebugToolConnection.h"
#include "RemoteDebugToolStatic.h"

//include Qt widgets that we use
#include <qaccel.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qprinter.h>
#include <qpushbutton.h>
#include <qstatusbar.h>
#include <qtimer.h>

#pragma warning(push, 3)
#include <qtextstream.h>
#pragma warning(pop)

#include <qtextview.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qwhatsthis.h>

//include the bitmaps for the toolbar
#include "res/filesave.xpm"
#include "res/fileopen.xpm"
#include "res/fileprint.xpm"
#include "res/application_icon.xpm"

const char * fileOpenText = "<img source=\"fileopen\"> "
"Click this button to open a <em>new file</em>. <br><br>"
"You can also select the <b>Open command</b> from the File menu.";

const char * fileSaveText = "Click this button to save the file you are "
"editing.  You will be prompted for a file name.\n\n"
"You can also select the Save command from the File menu.\n\n"
"Note that implementing this function is left as an exercise for the reader.";

const char * filePrintText = "Click this button to print the file you "
"are editing.\n\n"
"You can also select the Print command from the File menu.";

const char *serverEditText = "Enter the server to connect to in this edit box.";

const char *portEditText = "Enter the port to connect to in this edit box.";

const char *connectButtonText = "Once the server and port are set, press this to actually establish"
                                " a session with the server.";

const char *autoListenText = "Check this to have the application listen for connections started by a"
                             " server application.";

const char *prefsFileName = "prefs.txt";

/** Default constructor.  Besides creating a ChannelWindow (the main window of the app),
  * it also allocates a printer, and reads in the preference file.  It also creates  the
  * menus and toolbars, including the Connection toolbar.  It then hooks up the signals and
  * slots.
  */
ApplicationWindow::ApplicationWindow()
: QMainWindow( 0, "example application main window", WDestructiveClose ),
  m_printer(NULL),
  m_channelWindow(NULL),
  m_fileToolbar(NULL),
  m_filename(),
  m_connectToolbar(NULL),
  m_serverEdit(NULL),
  m_portEdit(NULL),
  m_connectButton(NULL),
  m_autoListenCheck(NULL),
  m_NetworkUpdateTimer(NULL),
  m_UpdateChannelTimer(NULL),
  m_prefsFile(NULL),
  m_updateNetworkTime(1),
  m_updateChannelTime(10000),
  m_connected(false),
  m_autoListen(true),
	m_theConnection(NULL)
{
	//set the application icon to a "Qt" symbol, replace with verant icon later if possible
	QPixmap appIcon(application_icon);
	setIcon(appIcon);

	//read in prefs
	m_prefsFile = new QFile(prefsFileName);
	if ( m_prefsFile->open(IO_ReadOnly) )
	{
		QTextStream t( m_prefsFile );
		QString str;
		while ( !t.eof() ) 
		{
			//read a line of text excluding '\n'
			str = t.readLine();                
		}
		m_prefsFile->close();
	}

	//create a printer to print to
	m_printer = new QPrinter;


	RemoteDebugToolStatic::install(this);

	//create file toolbar
	QPixmap openIcon, saveIcon, printIcon;
	m_fileToolbar = new QToolBar( this, "file operations" );
	m_fileToolbar->setLabel( tr( "File Operations" ) );
	openIcon = QPixmap( fileopen );
	QToolButton * fileOpen  = new QToolButton( openIcon, "Open File", QString::null, this, SLOT(load()), m_fileToolbar, "open file" );
	saveIcon = QPixmap( filesave );
	QToolButton * fileSave  = new QToolButton( saveIcon, "Save File", QString::null, this, SLOT(save()), m_fileToolbar, "save file" );
	printIcon = QPixmap( fileprint );
	QToolButton * filePrint = new QToolButton( printIcon, "Print File", QString::null, this, SLOT(print()), m_fileToolbar, "print file" );
	static_cast<void>(QWhatsThis::whatsThisButton(m_fileToolbar));
	QWhatsThis::add( fileOpen, fileOpenText );
	QMimeSourceFactory::defaultFactory()->setPixmap( "fileopen", openIcon );
	QWhatsThis::add( fileSave, fileSaveText );
	QWhatsThis::add( filePrint, filePrintText );

	//create the network connection toolbar
	m_connectToolbar = new QToolBar(this, "network connection operations");
	m_connectToolbar->setLabel(tr("Network Connection Operations"));
	QLabel *serverlabel = new QLabel(" Server: ", m_connectToolbar);
	m_serverEdit = new QLineEdit(m_connectToolbar);
	//TODO read this from a pref file
	m_serverEdit->setText("localhost");

	QLabel *portlabel = new QLabel("   Port: ", m_connectToolbar);
	m_portEdit = new QLineEdit(m_connectToolbar);
	m_portEdit->setMaximumWidth(50);
	//TODO read this from a pref file
	m_portEdit->setText("4445");
	QLabel *blanklabel = new QLabel("   ", m_connectToolbar);
	UNREF(blanklabel);
	m_connectButton = new QPushButton("Connect", m_connectToolbar);
	QLabel *blanklabel2 = new QLabel("   ", m_connectToolbar);
	UNREF(blanklabel2);
	m_autoListenCheck = new QCheckBox("Auto-listen", m_connectToolbar);
	//TODO read this from a pref file
	m_autoListenCheck->setChecked(true);
	QWhatsThis::add( serverlabel, serverEditText );
	QWhatsThis::add( m_serverEdit, serverEditText );
	QWhatsThis::add( portlabel, portEditText );
	QWhatsThis::add( m_portEdit, portEditText );
	QWhatsThis::add( m_connectButton, connectButtonText);
	QWhatsThis::add( m_autoListenCheck , autoListenText);

	//create the file menu
	int id;
	QPopupMenu *file = new QPopupMenu( this );
	menuBar()->insertItem( "&File", file );
	file->insertItem( "&New", this, SLOT(newDoc()), CTRL+Key_N );
	id = file->insertItem( openIcon, "&Open", this, SLOT(load()), CTRL+Key_O );
	file->setWhatsThis( id, fileOpenText );
	id = file->insertItem( saveIcon, "&Save", this, SLOT(save()), CTRL+Key_S );
	file->setWhatsThis( id, fileSaveText );
	id = file->insertItem( "Save &as...", this, SLOT(saveAs()) );
	file->setWhatsThis( id, fileSaveText );
	file->insertSeparator();
	id = file->insertItem( printIcon, "&Print", this, SLOT(print()), CTRL+Key_P );
	file->setWhatsThis( id, filePrintText );
	file->insertSeparator();
	file->insertItem( "&Close", this, SLOT(close()), CTRL+Key_W );
	file->insertItem( "E&xit", qApp, SLOT( closeAllWindows() ), CTRL+Key_Q );

	//create the help menu
	QPopupMenu *help = new QPopupMenu( this );
	menuBar()->insertSeparator();
	menuBar()->insertItem( "&Help", help );
	help->insertItem( "&About", this, SLOT(about()), Key_F1 );
	help->insertItem( "About &Qt", this, SLOT(aboutQt()) );
	help->insertSeparator();
	help->insertItem( "What's &This", this, SLOT(whatsThis()), SHIFT+Key_F1 );

/////
	m_remoteDebugToolService = new Service(ConnectionAllocator<RemoteDebugToolConnection>(), 4444, 100);

	//check the network every m_updateNetworkTime milliseconds
	m_NetworkUpdateTimer = new QTimer(this);
	m_NetworkUpdateTimer->start( m_updateNetworkTime, true );

	//check for a new channel list every m_updateChannelTime milliseconds
	m_UpdateChannelTimer = new QTimer(this);
	m_UpdateChannelTimer->start( m_updateChannelTime, true );

/////

	//actually create the main window and display it
	m_channelWindow = new ChannelWindow(this);
//	m_channelWindow->setFocus();
	setCentralWidget( m_channelWindow );
	statusBar()->message( "Ready", 2000 );

//////

	//connect the signals and slots
	connect(m_connectButton,      SIGNAL(clicked()), this, SLOT(slotConnectButtonClicked()));
	connect(m_NetworkUpdateTimer, SIGNAL(timeout()), this, SLOT(slotNetworkUpdateTimerDone()));
	connect(m_UpdateChannelTimer, SIGNAL(timeout()), this, SLOT(slotChannelUpdateTimerDone()));
	connect(m_autoListenCheck,    SIGNAL(clicked()), this, SLOT(slotAutoListenCheckClicked()));

	RemoteDebugClient::install(NULL, RemoteDebugToolStatic::open, RemoteDebugToolStatic::connectionClosed, RemoteDebugToolStatic::send, RemoteDebugToolStatic::isReady, RemoteDebugToolStatic::newStream, RemoteDebugToolStatic::messageOnStream, RemoteDebugToolStatic::newVariable, RemoteDebugToolStatic::variableValue, RemoteDebugToolStatic::variableType, RemoteDebugToolStatic::beginFrame, RemoteDebugToolStatic::endFrame, RemoteDebugToolStatic::newStaticView, RemoteDebugToolStatic::messageOnStaticView);
}

// -----------------------------------------------------------------

/** Destructor. Cleans up the allocated memory (though Qt should delete all children objects
  * of this object.  Delete non-child objects like the printer.
  */
ApplicationWindow::~ApplicationWindow()
{
	RemoteDebugClient::remove();

	if (m_printer)
		delete m_printer;
	if (m_prefsFile)
		delete m_prefsFile;

	if (m_theConnection)
		delete m_theConnection;
	delete m_remoteDebugToolService;
}

// -----------------------------------------------------------------

/** Function called when the app tries to exit.  We could have a dialog
  * ask about saving, etc, but it isn't necessary, so we trivially accept it.
  */
void ApplicationWindow::closeEvent( QCloseEvent* ce )
{
	ce->accept();
}

// -----------------------------------------------------------------

/** Called when we have a new stream.  Simply forwards it on to the ChannelWindow
  * which actually handles it.
  */
void ApplicationWindow::newStream(uint32 streamNumber, const char *newStreamName)
{
	m_channelWindow->newStream(streamNumber, newStreamName);
}

// -----------------------------------------------------------------

/** Called when we have a message on a stream.  Simply forwards it on to the ChannelWindow
  * which actually handles it.
  */
void ApplicationWindow::messageOnStream(uint32 streamNumber, const char *message)
{
	m_channelWindow->messageOnStream(streamNumber, message);
}

// -----------------------------------------------------------------

/** Called when we have a new static view.  Simply forwards it on to the ChannelWindow
  * which actually handles it.
  */
void ApplicationWindow::newStaticView(uint32 staticViewNumber, const char *newChannelName)
{
	m_channelWindow->newStaticView(staticViewNumber, newChannelName);
}

// -----------------------------------------------------------------

/** Called when we have a request to begin the frame of a static view.  
  * Simply forwards it on to the ChannelWindow which actually handles it.
  */
void ApplicationWindow::beginFrame(uint32 staticViewNumber)
{
	m_channelWindow->beginFrame(staticViewNumber);
}

// -----------------------------------------------------------------

/** Called when we have a request to end the frame of a static view.  
  * Simply forwards it on to the ChannelWindow which actually handles it.
  */
void ApplicationWindow::endFrame(uint32 staticViewNumber)
{
	m_channelWindow->endFrame(staticViewNumber);
}

// -----------------------------------------------------------------

/** Called when we have a message on a static view.  Simply forwards it on to the ChannelWindow
  * which actually handles it.
  */
void ApplicationWindow::messageOnStaticView(uint32 staticViewNumber, const char *message)
{
	m_channelWindow->messageOnStaticView(staticViewNumber, message);
}

// -----------------------------------------------------------------

/** Called when we have a new variable.  Simply forwards it on to the ChannelWindow
  * which actually handles it.
  */
void ApplicationWindow::newVariable(uint32 variableNumber, const char *newVariableName)
{
	m_channelWindow->newVariable(variableNumber, newVariableName);
}
// -----------------------------------------------------------------

/** Called when we have a variable type.  Simply forwards it on to the ChannelWindow
  * which actually handles it.
  */
void ApplicationWindow::variableType(uint32 variableNumber, RemoteDebug::VARIABLE_TYPES type)
{
	m_channelWindow->variableType(variableNumber, type);
}
// -----------------------------------------------------------------

/** Called when we have a variable value.  Simply forwards it on to the ChannelWindow
  * which actually handles it.
  */
void ApplicationWindow::variableValue(uint32 variableNumber, const char *varValue)
{
	m_channelWindow->variableValue(variableNumber, varValue);
}
// -----------------------------------------------------------------
/** Called when we have a network connection that is ready to accept data.
  * That is a our signal to change the button text and set our connected flag
  */
void ApplicationWindow::isReady()
{
	m_connectButton->setText("Disconnect");

	m_channelWindow->getEditor()->append("Connection established");
	//ensure that the new line is visible
	QScrollBar *verticalScrollBar = m_channelWindow->getEditor()->verticalScrollBar();
	int maxRange = verticalScrollBar->maxValue();
	verticalScrollBar->setValue(maxRange);

	m_connected = true;
	m_channelWindow->initFolders();
}

// -----------------------------------------------------------------

/** Called when a network connection goes down.  We change the button text
  * and our connection status variable
  */
void ApplicationWindow::connectionClosed()
{
	m_connectButton->setText("Connect");

	m_channelWindow->getEditor()->append("Connection closed");
	//ensure that the new line is visible
	QScrollBar *verticalScrollBar = m_channelWindow->getEditor()->verticalScrollBar();
	int maxRange = verticalScrollBar->maxValue();
	verticalScrollBar->setValue(maxRange);

	m_connected = false;
}

//end regular member functions
// -----------------------------------------------------------------
//Begin slots

/** Called from the menu.  Gives a simple explanation about the app
  */
void ApplicationWindow::slotAbout()
{
	QMessageBox::about( this, "ChannelViewer",
	                    "This application shows debug output generated "
	                    "by applications using the DebugChannel interface.");
}

// -----------------------------------------------------------------

/** Called from the menu.  Gives a simple explanation about Qt
  */
void ApplicationWindow::slotAboutQt()
{
	QMessageBox::aboutQt( this, "ChannelViewer" );
}

// -----------------------------------------------------------------

/** Called when we click on the Connect/Disconnet button.  This either sends
  * a connection or disconnecti on request.
  */
void ApplicationWindow::slotConnectButtonClicked()
{
	if(!m_connected)
	{
		//get server and port from UI
		QString server = m_serverEdit->text();
		QString portStr = m_portEdit->text();
		int portInt = static_cast<int>(atof(portStr));
		RemoteDebugClient::open(server, static_cast<uint32>(portInt));
	}
	else
	{
		RemoteDebugClient::close();
	}
}

// -----------------------------------------------------------------

/** Called repeatedly to update the network.  This function sets a timer to
  * call itself again in m_updateNetworkTime milliseconds.
  */
void ApplicationWindow::slotNetworkUpdateTimerDone()
{
	NetworkHandler::update();
	NetworkHandler::dispatch();
	m_NetworkUpdateTimer->start(static_cast<int32>(m_updateNetworkTime), true);
}

// -----------------------------------------------------------------

/** Called repeatedly to update the channel list.  This function sets a timer to
  * call itself again in m_updateChannelTime milliseconds.
  */
void ApplicationWindow::slotChannelUpdateTimerDone()
{
	RemoteDebugClient::send(RemoteDebug::REQUEST_ALL_CHANNELS, "");
	m_UpdateChannelTimer->start(static_cast<int32>(m_updateChannelTime), true);
}

// -----------------------------------------------------------------

/** Called by clicking on the "auto-listen" checkbox.  This sets the variable
  * that defines whether we listen for local connections or not.
  */
void ApplicationWindow::slotAutoListenCheckClicked()
{
	m_autoListen = m_autoListenCheck->isOn();
}

// -----------------------------------------------------------------

void ApplicationWindow::setTheConnection(RemoteDebugToolConnection* c)
{
	m_theConnection = c;
}

// -----------------------------------------------------------------

void ApplicationWindow::send(void *buffer, uint32 bufferLen)
{
	if(m_theConnection)
	{
		Archive::ByteStream bs(static_cast<const unsigned char *const>(buffer), bufferLen);
		m_theConnection->send(bs, true);
	}
}

// -----------------------------------------------------------------

void ApplicationWindow::open(const char *server, uint16 port)
{
	if(m_theConnection)
		delete m_theConnection;
	m_theConnection = new RemoteDebugToolConnection(server, port);
}

// -----------------------------------------------------------------
