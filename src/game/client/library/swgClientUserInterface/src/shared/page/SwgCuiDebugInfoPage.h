//======================================================================
//
// SwgCuiDebugInfoPage.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiDebugInfoPage_H
#define INCLUDED_SwgCuiDebugInfoPage_H

//======================================================================

#include "UINotification.h"
#include "UIEventCallback.h"
#include "sharedMath/Transform.h"
#include "sharedMessageDispatch/Receiver.h"
#include "clientUserInterface/CuiMediator.h"

class CellProperty;
class Object;
class UIDataSource;
class UIPage;
class UIText;
class UIWidget;

// ======================================================================
/**
* SwgCuiDebugInfoPage
*/

class SwgCuiDebugInfoPage :
public CuiMediator,
public UINotification,
public UIEventCallback,
public MessageDispatch::Receiver
{
public:

	explicit SwgCuiDebugInfoPage (UIPage& page);

	virtual void Notify (UINotificationServer* notifyingObject, UIBaseObject* contextObject, UINotification::Code notificationCode);
	virtual void OnButtonPressed (UIWidget* context);
	virtual void receiveMessage (const MessageDispatch::Emitter& source, const MessageDispatch::MessageBase& message);

protected:

	virtual void performActivate ();
	virtual void performDeactivate ();

private:

	virtual ~SwgCuiDebugInfoPage ();

	void updatePositions ();
	void updatePositions (const Object* object, UIText* position_wText, UIText* position_pText) const;
	void updateFps ();
	void updateTerrain ();
	void updateBandwidth ();
	void updateServerInfo ();
	void updateAnimInfo ();

private:

	SwgCuiDebugInfoPage ();
	SwgCuiDebugInfoPage (const SwgCuiDebugInfoPage& rhs);
	SwgCuiDebugInfoPage& operator= (const SwgCuiDebugInfoPage& rhs);

private:

	UIText*             m_camera_pText;
	UIText*             m_camera_wText;
	UIText*             m_player_pText;
	UIText*             m_player_wText;
	UIText*             m_fpsText;
	UIText*             m_terrainText;
	UIText*             m_pingTimeText;
	UIText*             m_bandwidthText;
	UIText*             m_versionText;
	UIText*             m_serverLoopTimeText;
	UIText*				m_playerAnimInfoText;
	UIText*				m_playerAnimTrackText[7];
	UIText*             m_playerServerSpeed;
	UIText*             m_playerServerSpeedEnabled;
	
	const CellProperty* m_lastCameraCellProperty;
	Transform           m_lastCameraTransform_p;
	const CellProperty* m_lastPlayerCellProperty;
	Transform           m_lastPlayerTransform_p;

	float               m_lastFps;
	int                 m_lastFrameNumber;
	int                 m_cachedPing;
	bool                m_serverLoopTimeRequested;
};

//======================================================================

#endif

