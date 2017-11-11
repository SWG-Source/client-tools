//======================================================================
//
// SwgCuiNotifications.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiNotifications_H
#define INCLUDED_SwgCuiNotifications_H

//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiNotifications.h"

#include "clientAudio/SoundId.h"
#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedObject/CachedNetworkId.h"
#include "UITypes.h"
#include "UILowerString.h"

#include <map>

class ClientNotificationBoxMessage;

namespace MessageDispatch
{
	class Emitter;
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiNotifications :
public CuiMediator,
public UIEventCallback,
public MessageDispatch::Receiver
{

	struct NotificationRecord
	{
		UIPage *iconPage;
		UIPage *notificationPage;
		Unicode::String contents;
		int sequenceId;
		float timeout;
		int verticalPosition;
		int iconStyle;
		SoundId soundId;
		std::string sound;
	};

public:
	explicit            SwgCuiNotifications (UIPage & page, int sceneType);

	virtual bool        OnMessage                    (UIWidget *context, const UIMessage & msg );
	virtual void        receiveMessage               (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);
			void        update                       (float deltaTimeSecs);

protected:
	
	virtual void        performActivate              ();
	virtual void        performDeactivate            ();
	
private:
	
	void                addNotification              (const ClientNotificationBoxMessage &cnbm);
	void                closeNotification            (int sequenceId);
	void                closeAllNotifications        ();

	void                positionPage                 (UIPage *page, int verticalPosition);
	void                positionIconPage             (UIPage *page, int verticalPosition);

	void                turnIconIntoNotification     (NotificationRecord &nr);
	void                ensureOnlyTopIconIsVisible   ();
	void                ensureOnlyTopPageIsVisible   ();

	                    SwgCuiNotifications          ();
	                    SwgCuiNotifications          (const SwgCuiNotifications &);
	SwgCuiNotifications &operator=                   (const SwgCuiNotifications &);

	                   ~SwgCuiNotifications          ();
	
	UIPage *					m_sampleIconPage;
	UIPage *					m_sampleNotificationPage;

   std::map<int, NotificationRecord> m_activeNotificationsMap;

	int                         m_numNotificationsActive;
	int							m_numIconsActive;	
	int                         m_sceneType;
	int                         m_closeThisNotificationPageNextUpdate;
};
//======================================================================

#endif
