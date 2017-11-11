//======================================================================
//
// SwgCuiNotifications.cpp
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiNotifications.h"

#include "clientAudio/Audio.h"
#include "clientGame/Game.h"
#include "clientGraphics/ConfigClientGraphics.h"
#include "clientUserInterface/CuiPreferences.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/ClientNotificationBoxMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIImage.h"
#include "UIMessage.h"
#include "UIText.h"

#include <vector>

namespace SwgCuiNotificationsNamespace
{
	const UILowerString SEQUENCE_ID_PROPERTY = UILowerString("nseq");

	const float START_FADE = 1.0f;
}

using namespace SwgCuiNotificationsNamespace;

//----------------------------------------------------------------------

SwgCuiNotifications::SwgCuiNotifications (UIPage & page, int sceneType) :
CuiMediator         ("SwgCuiNotifications", page),
UIEventCallback     (),
MessageDispatch::Receiver (),
m_sampleIconPage(0),
m_sampleNotificationPage(0),
m_activeNotificationsMap(),
m_numNotificationsActive(0),
m_numIconsActive(0),
m_sceneType(sceneType),
m_closeThisNotificationPageNextUpdate(-1)
{
	getCodeDataObject (TUIPage, m_sampleIconPage,      "hinticon", true);
	getCodeDataObject (TUIPage, m_sampleNotificationPage, "hint", true);

	if(m_sampleIconPage)
		m_sampleIconPage->SetVisible(false);
	if(m_sampleNotificationPage)
		m_sampleNotificationPage->SetVisible(false);
	
	connectToMessage (ClientNotificationBoxMessage::MessageType);
}


//----------------------------------------------------------------------

SwgCuiNotifications::~SwgCuiNotifications ()
{
	m_sampleIconPage = 0;
	m_sampleNotificationPage = 0;

	closeAllNotifications();
	
	deactivate();
}

//----------------------------------------------------------------------

bool SwgCuiNotifications::OnMessage( UIWidget *context, const UIMessage & msg )
{

	NON_NULL(context);
	if (msg.Type == UIMessage::LeftMouseUp)
	{
		int sequenceId;
		if(context->GetPropertyInteger(SEQUENCE_ID_PROPERTY, sequenceId))
		{
			if(sequenceId != 0)
			{
				m_closeThisNotificationPageNextUpdate = sequenceId;
				return false;
			}
		}
		std::map<int, NotificationRecord>::iterator activeI;
		for(activeI = m_activeNotificationsMap.begin(); activeI != m_activeNotificationsMap.end(); ++activeI)
		{
			NotificationRecord &nrLoop = activeI->second;
			if(nrLoop.iconPage == context)
			{
				turnIconIntoNotification(nrLoop);
				return false;
			}
		}
		return true;
	}
	return true;
}

//----------------------------------------------------------------------
void SwgCuiNotifications::closeNotification(int sequenceId)
{
	std::map<int, NotificationRecord>::iterator activeI = m_activeNotificationsMap.find(sequenceId);
	if(activeI == m_activeNotificationsMap.end())
		return;
	NotificationRecord const &nr = activeI->second;
	int saveVerticalPosition = 999;
	if(nr.notificationPage)
	{
		//Check for deselected checkbox
		UICheckbox *checkbox = NON_NULL (GET_UI_OBJ ((*nr.notificationPage), UICheckbox, "notificationHintsCheck"));
		if(!checkbox->IsChecked())
		{
			//User wants to turn it off
			CuiPreferences::setShowNotifications(false);
		}

		nr.notificationPage->SetVisible(false);

		//Unregister the close and cancel buttons
		UIButton *cancelButton = NON_NULL (GET_UI_OBJ ((*nr.notificationPage), UIButton, "btnCancel"));
		unregisterMediatorObject(*cancelButton);
		UIButton *closeButton = NON_NULL (GET_UI_OBJ ((*nr.notificationPage), UIButton, "close"));
		unregisterMediatorObject(*closeButton);

		getPage().RemoveChild(nr.notificationPage);
		saveVerticalPosition = nr.verticalPosition;
	}
	if(!nr.sound.empty() && Audio::isSoundPlaying(nr.soundId))
	{
		Audio::stopSound(nr.soundId, 0.5f);
	}
	m_activeNotificationsMap.erase(activeI);
	for(activeI = m_activeNotificationsMap.begin(); activeI != m_activeNotificationsMap.end(); ++activeI)
	{
		NotificationRecord &nrLoop = activeI->second;
		if(nrLoop.notificationPage && (nrLoop.verticalPosition > saveVerticalPosition))
		{
			nrLoop.verticalPosition--;
			positionPage(nrLoop.notificationPage, nrLoop.verticalPosition);			
		}
	}
	m_numNotificationsActive--;
	ensureOnlyTopPageIsVisible();
	return;
}

//-----------------------------------------------------------------

void SwgCuiNotifications::receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
{
	if(Game::getHudSceneType() != static_cast<int>(m_sceneType))
		return;
	if (message.isType (ClientNotificationBoxMessage::MessageType))
	{
		//-- what type of request is it?
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage*> (&message))->getByteStream ().begin ();
		const ClientNotificationBoxMessage cnbm(ri);
		if(cnbm.getChannel() == ClientNotificationBoxMessage::NC_SPECIAL_CANCEL)
			closeNotification(cnbm.getSequenceId());
		else if(cnbm.getChannel() == ClientNotificationBoxMessage::NC_SPECIAL_CANCEL_ALL)
			closeAllNotifications();
		else
			addNotification(cnbm);
	}
}

//----------------------------------------------------------------------

void SwgCuiNotifications::performActivate()
{
	setIsUpdating(true);
}

//----------------------------------------------------------------------

void SwgCuiNotifications::performDeactivate()
{
	setIsUpdating(false);
	closeAllNotifications();	
}

//----------------------------------------------------------------------

void SwgCuiNotifications::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	if(m_closeThisNotificationPageNextUpdate != -1)
	{
		closeNotification(m_closeThisNotificationPageNextUpdate);
		m_closeThisNotificationPageNextUpdate = -1;
	}

	std::map<int, NotificationRecord>::iterator activeI;
	std::vector<int> idsToErase;
	for(activeI = m_activeNotificationsMap.begin(); activeI != m_activeNotificationsMap.end(); ++activeI)
	{
		NotificationRecord &nrLoop = activeI->second;
		if((nrLoop.notificationPage != NULL) && (nrLoop.timeout > 0.0f))
		{
			nrLoop.timeout -= deltaTimeSecs;
			if(nrLoop.timeout < 0.0f)
			{
				idsToErase.push_back(nrLoop.sequenceId);
			}
			else if(nrLoop.timeout < START_FADE)
			{
				nrLoop.notificationPage->SetOpacity(nrLoop.timeout / START_FADE);
			}
		}
	}
	for(std::vector<int>::iterator it = idsToErase.begin(); it != idsToErase.end(); ++it)
	{
		closeNotification(*it);
	}
}

//-----------------------------------------------------------------

void SwgCuiNotifications::addNotification(const ClientNotificationBoxMessage &cnbm)
{
	if(!CuiPreferences::getShowNotifications())
		return;
	
	int sequenceId = cnbm.getSequenceId();
	Unicode::String const & contents = cnbm.getContents();
	bool useNotificationIcon = cnbm.getUseNotificationIcon();
	int iconStyle = cnbm.getIconStyle();
	float timeout = cnbm.getTimeout();
	std::string const & sound = cnbm.getSound();
	
	if(!useNotificationIcon)
	{	
		UIPage *newNotificationPage = NON_NULL (static_cast<UIPage *>(m_sampleNotificationPage->DuplicateObject ())); //lint !e1774 //stfu noob
		UIText *newNotificationTextPage = NON_NULL (GET_UI_OBJ ((*newNotificationPage), UIText, "text"));

		getPage().AddChild(newNotificationPage);
		newNotificationPage->Link ();

		Unicode::String localContents = (iconStyle == ClientNotificationBoxMessage::IS_NONE) ? Unicode::emptyString : Unicode::narrowToWide("\n\n\n");
		localContents.append(contents);
		newNotificationTextPage->SetText(localContents);
		newNotificationPage->SetVisible(true);

		//Add the new page to the list
		NotificationRecord nr;
		nr.iconPage = NULL;
		nr.sequenceId =  sequenceId;
		nr.iconStyle = iconStyle;
		nr.notificationPage = newNotificationPage;
		nr.timeout = timeout;
		nr.verticalPosition = m_numNotificationsActive++;
		nr.sound = sound;

			
		if(!sound.empty())
		{
			nr.soundId = Audio::playSound(sound.c_str(), NULL);			
		}

		m_activeNotificationsMap.insert(std::make_pair(sequenceId, nr));

		//Hook up the close and cancel buttons
		UIButton *cancelButton = NON_NULL (GET_UI_OBJ ((*newNotificationPage), UIButton, "btnCancel"));
		cancelButton->SetPropertyInteger(SEQUENCE_ID_PROPERTY, sequenceId);
		registerMediatorObject(*cancelButton, true);
		UIButton *closeButton = NON_NULL (GET_UI_OBJ ((*newNotificationPage), UIButton, "close"));
		closeButton->SetPropertyInteger(SEQUENCE_ID_PROPERTY, sequenceId);
		registerMediatorObject(*closeButton, true);

		UIImage *exclamation =  NON_NULL (GET_UI_OBJ ((*newNotificationPage), UIImage, "exclamation"));
		UIImage *question = NON_NULL (GET_UI_OBJ ((*newNotificationPage), UIImage, "question"));
		
		exclamation->SetVisible(iconStyle == ClientNotificationBoxMessage::IS_EXCLAMATION);
		question->SetVisible(iconStyle == ClientNotificationBoxMessage::IS_QUESTION);
	
		positionPage(newNotificationPage, nr.verticalPosition);
			
		ensureOnlyTopPageIsVisible();
	}
	else
	{
		//Then get the icon hooked up
		UIPage *newNotificationPage = NON_NULL (static_cast<UIPage *>(m_sampleIconPage->DuplicateObject ())); //lint !e1774 //stfu noob
		getPage().AddChild(newNotificationPage);
		newNotificationPage->Link ();
		newNotificationPage->SetVisible(true);

		NotificationRecord nr;
		nr.notificationPage = NULL;
		nr.sequenceId = sequenceId;
		nr.iconStyle = iconStyle;
		nr.iconPage = newNotificationPage;
		nr.timeout = timeout;
		nr.verticalPosition = m_numIconsActive++;
		nr.contents = contents;
		nr.sound = sound;

		m_activeNotificationsMap.insert(std::make_pair(sequenceId, nr));
		registerMediatorObject(*newNotificationPage, true);
		positionIconPage(newNotificationPage, nr.verticalPosition);		
		ensureOnlyTopIconIsVisible();
	}

	
}

//-----------------------------------------------------------------

void SwgCuiNotifications::positionPage(UIPage *page, int verticalPosition)
{
	int width = page->GetWidth();
	int height = page->GetHeight();
	int screenHeight = ConfigClientGraphics::getScreenHeight();
	int screenWidth = ConfigClientGraphics::getScreenWidth();
	UIPoint newPosition(screenWidth - width - 5, screenHeight - (screenHeight / 5) - (height + 25) * (verticalPosition + 1));
	page->SetLocation(newPosition);
}


//-----------------------------------------------------------------

void SwgCuiNotifications::positionIconPage(UIPage *page, int verticalPosition)
{
	int width = page->GetWidth();
	int height = page->GetHeight();
	int screenHeight = ConfigClientGraphics::getScreenHeight();
	int screenWidth = ConfigClientGraphics::getScreenWidth();
	UIPoint newPosition(screenWidth - (width + 5) * (verticalPosition + 1) - 60, screenHeight - (height + 15));
	page->SetLocation(newPosition);
}


//-----------------------------------------------------------------

void SwgCuiNotifications::turnIconIntoNotification(NotificationRecord &nr)
{
	//Remove icon page
	int saveVerticalPosition = 999;
	if(nr.iconPage)
	{
		nr.iconPage->SetVisible(false);
		getPage().RemoveChild(nr.iconPage);
		nr.iconPage = 0;
		saveVerticalPosition = nr.verticalPosition;
		m_numIconsActive--;
	}
	//Move icons down
	std::map<int, NotificationRecord>::iterator activeI;
	for(activeI = m_activeNotificationsMap.begin(); activeI != m_activeNotificationsMap.end(); ++activeI)
	{
		NotificationRecord &nrLoop = activeI->second;
		if(nrLoop.iconPage && !nrLoop.notificationPage && nrLoop.verticalPosition > saveVerticalPosition)
		{
			nrLoop.verticalPosition--;
			positionIconPage(nrLoop.iconPage, nrLoop.verticalPosition);
		}
	}
	//Add a notification page
	UIPage *newNotificationPage = NON_NULL (static_cast<UIPage *>(m_sampleNotificationPage->DuplicateObject ())); //lint !e1774 //stfu noob
	UIText *newNotificationTextPage = NON_NULL (GET_UI_OBJ ((*newNotificationPage), UIText, "text"));

	getPage().AddChild(newNotificationPage);
	newNotificationPage->Link ();

	Unicode::String localContents = (nr.iconStyle == ClientNotificationBoxMessage::IS_NONE) ? Unicode::emptyString : Unicode::narrowToWide("\n\n\n");
	localContents.append(nr.contents);
	newNotificationTextPage->SetText(localContents);
	newNotificationPage->SetVisible(true);

	nr.notificationPage = newNotificationPage;
	nr.verticalPosition = m_numNotificationsActive++;

	//Hook up the close and cancel buttons
	UIButton *cancelButton = NON_NULL (GET_UI_OBJ ((*newNotificationPage), UIButton, "btnCancel"));
	cancelButton->SetPropertyInteger(SEQUENCE_ID_PROPERTY, nr.sequenceId);
	registerMediatorObject(*cancelButton, true);
	UIButton *closeButton = NON_NULL (GET_UI_OBJ ((*newNotificationPage), UIButton, "close"));
	closeButton->SetPropertyInteger(SEQUENCE_ID_PROPERTY, nr.sequenceId);
	registerMediatorObject(*closeButton, true);


	UIImage *exclamation =  NON_NULL (GET_UI_OBJ ((*newNotificationPage), UIImage, "exclamation"));
	UIImage *question = NON_NULL (GET_UI_OBJ ((*newNotificationPage), UIImage, "question"));
	
	exclamation->SetVisible(nr.iconStyle == ClientNotificationBoxMessage::IS_EXCLAMATION);
	question->SetVisible(nr.iconStyle == ClientNotificationBoxMessage::IS_QUESTION);
		
	if(!nr.sound.empty())
	{
		nr.soundId = Audio::playSound(nr.sound.c_str(), NULL);
		DEBUG_WARNING(true, ("SCN: setting sound id to %d %d", nr.soundId.getId(), Audio::isSoundPlaying(nr.soundId)));			
	}
	
	positionPage(newNotificationPage, nr.verticalPosition);
	ensureOnlyTopIconIsVisible();
	ensureOnlyTopPageIsVisible();
}

//-----------------------------------------------------------------

void SwgCuiNotifications::closeAllNotifications()
{
	while(!m_activeNotificationsMap.empty())
	{
		closeNotification(m_activeNotificationsMap.begin()->second.sequenceId);
	}
}

//-----------------------------------------------------------------

void SwgCuiNotifications::ensureOnlyTopIconIsVisible()
{
	std::map<int, NotificationRecord>::iterator activeI;
	for(activeI = m_activeNotificationsMap.begin(); activeI != m_activeNotificationsMap.end(); ++activeI)
	{
		NotificationRecord &nrLoop = activeI->second;
		if(nrLoop.iconPage && !nrLoop.notificationPage)
		{
			nrLoop.iconPage->SetVisible(nrLoop.verticalPosition == 0);		
		}
	}
}

//-----------------------------------------------------------------

void SwgCuiNotifications::ensureOnlyTopPageIsVisible()
{
	std::map<int, NotificationRecord>::iterator activeI;
	for(activeI = m_activeNotificationsMap.begin(); activeI != m_activeNotificationsMap.end(); ++activeI)
	{
		NotificationRecord &nrLoop = activeI->second;
		if(nrLoop.notificationPage && !nrLoop.iconPage)
		{
			nrLoop.notificationPage->SetVisible(nrLoop.verticalPosition == 0);		
		}
	}
}