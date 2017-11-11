// ======================================================================
//
// ApplicationWindow.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================


#ifndef ApplicationWindow_H
#define ApplicationWindow_H

#include "FirstRemoteDebugTool.h"
//#include "sharedMessageDispatch/Receiver.h"

#include <qmainwindow.h> //base for this class

// -----------------------------------------------------------------
//Forward declare when possible
class ChannelWindow;
class Connection;
class QMultiLineEdit;
class QToolBar;
class QPopupMenu;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QFile;
class RemoteDebugToolConnection;
class Service;

// -----------------------------------------------------------------

/** This class represents the main window (or Widget, in Qt-speak) of the application.
  * It owns the ChannelWindow, toolbars, menus, etc., and access to the RemoteDebug class 
  * (which sends and receives the packets).
  */
class ApplicationWindow: public QMainWindow//, public MessageDispatch::Receiver
{
	//necessary to make it a Qt object
	Q_OBJECT

	enum RemoteDebug::VARIABLE_TYPES;
	
public:
	ApplicationWindow();
	virtual ~ApplicationWindow();
	
public:
	void newStream(uint32 streamNumber, const char *newStreamName);
	void messageOnStream(uint32 streamNumber, const char *message);
	void newVariable(uint32 variableNumber, const char *newVariableName);
	void variableType(uint32 variableNumber, RemoteDebug::VARIABLE_TYPES type);
	void variableValue(uint32 variableNumber, const char *value);
	void newStaticView(uint32 staticViewNumber, const char *newStaticViewName);
	void messageOnStaticView(uint32 staticViewNumber, const char *message);
	void beginFrame(uint32 staticViewNumber);
	void endFrame(uint32 staticViewNumber);
	void isReady();
	void connectionClosed();
	void setTheConnection(RemoteDebugToolConnection* c);
	void send(void *buffer, uint32 bufferLen);
	void open(const char *server, uint16 port);

protected:
	void closeEvent(QCloseEvent*);

private slots:
	void slotAbout();
	void slotAboutQt();
	void slotConnectButtonClicked();
	void slotNetworkUpdateTimerDone();
	void slotChannelUpdateTimerDone();
	void slotAutoListenCheckClicked();

private:
	///An object we can print to
	QPrinter      *m_printer;
	///Our main window, holds the treeview and textview
	ChannelWindow *m_channelWindow;
	///The toolbar with file operations
	QToolBar      *m_fileToolbar;
	///The filename of the active file
	QString        m_filename;
	///The toolbar with network connection info
	QToolBar      *m_connectToolbar;
	///The server name edit box
	QLineEdit     *m_serverEdit;
	///The server port edit box
	QLineEdit     *m_portEdit;
	//The button to initiate connection with the server
	QPushButton   *m_connectButton;
	///The checkbox to determine whether to listen for incoming server startups
	QCheckBox     *m_autoListenCheck;
	///Timer used to periodically check the network for new traffic in and out
	QTimer        *m_NetworkUpdateTimer;
	///Timer used to periodically check for a new channel list
	QTimer        *m_UpdateChannelTimer;
	///The file where preferences are maintained
	QFile         *m_prefsFile;
	///time in milliseconds between network updates
	uint32         m_updateNetworkTime;
	///time in milliseconds between channel list updates
	uint32         m_updateChannelTime;
	///true if we have an open connection
	bool           m_connected;
	///true if the "auto listen" check is set
	bool           m_autoListen;

	Service *      m_remoteDebugToolService;

	Connection *   m_theConnection;

//disabled
private:
	ApplicationWindow (const ApplicationWindow&);
	ApplicationWindow& operator= (const ApplicationWindow&);
};

// -----------------------------------------------------------------

#endif // ApplicationWindow_H
