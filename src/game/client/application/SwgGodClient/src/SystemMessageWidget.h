// ======================================================================
//
// SystemMessageWidget.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SystemMessageWidget_H
#define INCLUDED_SystemMessageWidget_H

// ======================================================================

#include "BaseSystemMessageWidget.h"

// ======================================================================

//class QTimer;

// ======================================================================

class SystemMessageWidget : public BaseSystemMessageWidget, public MessageDispatch::Receiver
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	explicit SystemMessageWidget(QWidget *theParent=0, const char *theName=0);
	virtual ~SystemMessageWidget();

	void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

public slots:
    virtual void sendMessage();

private:
	//disabled
	SystemMessageWidget(const SystemMessageWidget& rhs);
	SystemMessageWidget& operator=(const SystemMessageWidget& rhs);
};

// ======================================================================

#endif
