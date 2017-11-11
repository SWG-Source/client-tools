// ======================================================================
//
// SystemMessageWidget.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"

#include "SystemMessageWidget.h"
#include "SystemMessageWidget.moc"

//#include "UnicodeUtils.h"
//#include "unicodeArchive/UnicodeArchive.h"

//#include "sharedFoundation/NetworkId.h"
//#include "sharedFoundation/NetworkIdArchive.h"

//#include "sharedNetworkMessages/MessageRegionListCircleResponse.h"
//#include "sharedNetworkMessages/MessageRegionListRectResponse.h"

//#include "RegionRenderer.h"
#include "ServerCommander.h"

#include <qbutton.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qradiobutton.h>

// ======================================================================

SystemMessageWidget::SystemMessageWidget(QWidget *theParent, const char *theName)
: BaseSystemMessageWidget (theParent, theName),
  MessageDispatch::Receiver ()
{
}

// ======================================================================

SystemMessageWidget::~SystemMessageWidget()
{
}

//-----------------------------------------------------------------------

/**
 * Todo move this to the brush list view, since it's a custom widget now and can handle this itself (connectToMessage too)
 */
void SystemMessageWidget::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	UNREF(source);
	UNREF(message);
}


//-----------------------------------------------------------------------

void SystemMessageWidget::sendMessage()
{
	std::string messageType;
	std::string targetName;
	std::string message;
	
	message = m_messageLineEdit->text().latin1();
	if(message.empty())
	{
		QMessageBox::information(this, "You must enter a message", "Please enter a message to send.");
		return;
	}

	QRadioButton* selectedRadioButton = dynamic_cast<QRadioButton*>(m_targetSelectionButtonGroup->selected());
	if(!selectedRadioButton)
	{
		//this shouldn't happen
		QMessageBox::warning(this, "Nothing selected", "Please select a radio button.");
		return;
	}

	if(selectedRadioButton == m_sendToPlayerRadioButton)
	{
		messageType = "Player";
		targetName = m_playerLineEdit->text().latin1();
	}
	else
	{
		messageType = "Channel";
		//TODO finish implementing this (channel selection)
		targetName = "SWG.EricDev.system";
	}

	ServerCommander::getInstance().sendSystemMessage(messageType, targetName, message);
	close();
}

// ======================================================================
