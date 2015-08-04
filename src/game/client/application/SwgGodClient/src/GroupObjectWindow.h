// ======================================================================
//
// GroupObjectWindow.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GroupObjectWindow_H
#define INCLUDED_GroupObjectWindow_H

// ======================================================================

#include "BaseGroupObjectWindow.h"

// ======================================================================

class GroupObjectWindow : public BaseGroupObjectWindow, public MessageDispatch::Receiver
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	explicit GroupObjectWindow(QWidget *theParent=0, const char *theName=0);
	virtual ~GroupObjectWindow();

	void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

private slots:
	void onBrushListSelectionChanged();

signals:
	void brushSelectionChanged(const std::string& name);

private:
	//disabled
	GroupObjectWindow(const GroupObjectWindow& rhs);
	GroupObjectWindow& operator=(const GroupObjectWindow& rhs);
};

// ======================================================================

#endif
