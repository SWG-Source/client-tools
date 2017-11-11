// ======================================================================
//
// TriggerWindow.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TriggerWindow_H
#define INCLUDED_TriggerWindow_H

// ======================================================================

#include "BaseTriggerWindow.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

// ======================================================================

class NetworkId;

// ======================================================================

class TriggerWindow : public BaseTriggerWindow, public MessageDispatch::Receiver
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	explicit TriggerWindow(QWidget *parent=0, const char *name=0);
	virtual ~TriggerWindow();
	void setNetworkId(const NetworkId& nid);
	void receiveMessage(const MessageDispatch::Emitter& source, const MessageDispatch::MessageBase& message);

private:
	NetworkId m_networkId;
};

// ======================================================================

#endif
