//======================================================================
//
// SwgCuiAvatarLocation2.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAvatarLocation2_H
#define INCLUDED_SwgCuiAvatarLocation2_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class CuiMessageBox;
class CuiTurntableAdapter;
class CuiWidget3dObjectListViewer;
class StartingLocationData;
class TangibleObject;
class UIButton;
class UIImage;
class UIImageStyle;
class UIList;
class UIPage;
class UIText;
class UIVolumePage;

namespace MessageDispatch
{
	class Callback;
}

// ======================================================================

class SwgCuiAvatarLocation2 :
public CuiMediator,
public UIEventCallback
{
public:

	typedef std::pair <StartingLocationData, bool>          LocationStatus;
	typedef stdvector<LocationStatus>::fwd                  LocationStatusVector;
	typedef stdmap<std::string, LocationStatusVector>::fwd  LocationStatusMap;

	explicit                 SwgCuiAvatarLocation2 (UIPage & page);

	void                     OnButtonPressed              (UIWidget * context);
	bool                     OnMessage                    (UIWidget * context, const UIMessage & msg);
	void                     OnVolumePageSelectionChanged (UIWidget * context);

//	virtual void             Notify            (UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code NotificationCode );

	virtual void             performActivate   ();
	virtual void             performDeactivate ();

	void                     setLocations                (const LocationStatusVector & lsv);
	void                     onStartingLocationsReceived (const LocationStatusVector & lsv);

	void                     onStartingLocationSelectionResult (const std::pair<std::string, bool> & payload);
	void                     onMessageBoxClosed                (const CuiMessageBox & box);
	
private:
	virtual                 ~SwgCuiAvatarLocation2 ();
	                         SwgCuiAvatarLocation2 ();
	                         SwgCuiAvatarLocation2 (const SwgCuiAvatarLocation2 & rhs);
	SwgCuiAvatarLocation2 &   operator=            (const SwgCuiAvatarLocation2 & rhs);

private:

	void                     setupPlanets          ();
	void                     selectPlanet          (int index, bool playSound);
	void                     updateDescription     ();
	const LocationStatus *   findLocationStatus    (const std::string & planetName, int cityIndex);
	void                     randomize             ();
	void                     clearPlanets          ();
	void                     ok                    ();

	UIButton *                    m_buttonBack;
	UIButton *                    m_buttonNext;

	UIVolumePage *                m_volumeCities;
	UIText *                      m_text;
	UIText *                      m_textPlanet;

	CuiWidget3dObjectListViewer * m_viewer;

	typedef stdvector<TangibleObject *>::fwd TangibleVector;
	TangibleVector *              m_planets;

	CuiTurntableAdapter *         m_turntable;

	LocationStatusMap *           m_locationStatusMap;
	LocationStatusVector *        m_locationStatusVector;

	std::string                   m_selectedLocation;
	bool                          m_selectedLocationAvailable;

	MessageDispatch::Callback *   m_callback;
	CuiMessageBox *               m_messageBox;

	bool                          m_isFinishing;
};

//======================================================================

#endif
