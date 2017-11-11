// ======================================================================
//
// ConsoleWindow.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleWindow_H
#define INCLUDED_ConsoleWindow_H

// ======================================================================

#include "BaseConsoleWindow.h"

// ======================================================================
/**
* ConsoleWindow outputs all console messages from the server.
*/
class ConsoleWindow : public BaseConsoleWindow, public MessageDispatch::Receiver
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	explicit ConsoleWindow(QWidget *parent=0, const char *name=0);
	virtual ~ConsoleWindow();

	virtual void              receiveMessage (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

public slots:
	void onConsoleCommandIssued (const char *);
};

// ======================================================================

#endif
