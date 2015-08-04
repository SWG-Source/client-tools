//======================================================================
//
// SwgCuiMissionDetails.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiMissionDetails_H
#define INCLUDED_SwgCuiMissionDetails_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedMessageDispatch/Receiver.h"
#include "clientGame/ClientWaypointObject.h"

class  ClientMissionObject;
class  UIComposite;
class  UIPage;
class  UIButton;
class  UIText;
class  CuiMessageBox;
class  MessageQueueMissionGenericResponse;
struct MessageQueueMissionDetailsResponseData;
class  MessageQueueMissionListResponse;
class  MessageQueueNetworkId;
class  NetworkId;
class  MessageQueueMissionListResponseData;
class  CuiWidget3dObjectListViewer;
class  ClientMissionObject;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiMissionDetails :
public CuiMediator,
public UIEventCallback,
public MessageDispatch::Receiver
{
public:
	static SwgCuiMissionDetails*  createInto         (UIPage & parent);

	explicit                SwgCuiMissionDetails (UIPage & page);

	virtual void            performActivate   ();
	virtual void            performDeactivate ();

	virtual bool            OnMessage( UIWidget *Context, const UIMessage & msg );
	virtual void            OnButtonPressed( UIWidget *context );

	virtual void            receiveMessage (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

	void                    updateDetails();
	void                    setDetails (const ClientMissionObject& mission);

private:
	void                    setDestroyDetails(const ClientMissionObject& mission);
	void                    setDeliverDetails(const ClientMissionObject& mission);
	void                    setBountyDetails (const ClientMissionObject& mission);
	void                    setSurveyDetails (const ClientMissionObject& mission);
	void                    setEntertainerDetails (const ClientMissionObject& mission);
	void                    setCraftingDetails    (const ClientMissionObject& mission);
	void                    setHuntingDetails     (const ClientMissionObject& mission);
	void                    setAssassinDetails    (const ClientMissionObject& mission);
	void                    onMissionAccept(const MessageQueueMissionGenericResponse&);
	void                    onMissionRemove(const MessageQueueMissionGenericResponse&);
	void                    onMissionAbort (const MessageQueueNetworkId&);
	void                    showDetailsPage(UIPage* page);
	void                    setSharedDetails(UIPage* page, const ClientMissionObject & details);
	void                    addCreatorToDetails(UIPage* page, const ClientMissionObject & details) const;
	void                    addRewardToDetails(UIPage* page,  const ClientMissionObject & details) const;
	void                    addTargetToDetails(UIPage* page,  const ClientMissionObject & details, bool pitch);
	void                    addStartLocationToDetails(UIPage* page) const;
	void                    addDestLocationToDetails(UIPage* page) const;
	void                    addDifficultyToDetails(UIPage* page, const ClientMissionObject & details) const;
	void                    clearDetails();

private:
//disabled
	                       ~SwgCuiMissionDetails ();
	                        SwgCuiMissionDetails (const SwgCuiMissionDetails &);
	SwgCuiMissionDetails &  operator= (const SwgCuiMissionDetails &);

private:
	UIPage *                     m_pageDestroy;
	UIPage *                     m_pageDeliver;
	UIPage *                     m_pageBounty;
	UIPage *                     m_pageSurvey;
	UIPage *                     m_pageEntertainer;
	UIPage *                     m_pageCrafting;
	UIPage *                     m_pageHunting;
	UIPage *                     m_pageAssassin;

	UIText *                     m_textDescription;
	UIText *                     m_textTitle;

	UIButton *                   m_buttonExit;
	UIButton *                   m_buttonAccept;
	UIButton *                   m_buttonAbort;
	UIButton *                   m_buttonWaypoint;

	CuiMessageBox *              m_messageBox;

	MessageDispatch::Callback *  m_callback;

	const ClientMissionObject*          m_missionObjectDetails;

	CuiWidget3dObjectListViewer* m_viewer;

	ClientObject *               m_target;
	Watcher<ClientWaypointObject> m_waypoint;
	float                        m_defaultViewerPitch;
};

//======================================================================

#endif
