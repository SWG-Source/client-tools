//======================================================================
//
// SwgCuiPlanetMap.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiPlanetMap_H
#define INCLUDED_SwgCuiPlanetMap_H

//======================================================================

#include "UIEventCallback.h"
#include "UITypes.h"
#include "clientGame/PlanetMapManagerClient.h"
#include "clientGame/GuildObject.h"
#include "clientUserInterface/CuiMediator.h"
#include "sharedMath/Vector.h"

class ClientObject;
class NetworkId;
class UIButton;
class UICheckbox;
class UIColorEffector;
class UIPopupMenu;
class UISliderbar;
class UIText;
class ClientWaypointObject;
class UITreeView;
class UIWidget;
class UIWidget;
class Vector2d;
class Vector;
struct UIMessage;
struct UIPoint;

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiPlanetMap :
public CuiMediator,
public UIEventCallback
{
public:

	explicit SwgCuiPlanetMap (UIPage & page);
	void onSceneChanged();

	enum ZoomLevel
	{
		ZL_Planet,
		ZL_Zone,
		ZL_ZoneSupermap
	};

protected:

	~SwgCuiPlanetMap();

	void                         performActivate           ();
	void                         performDeactivate         ();

	void                         OnButtonPressed           (UIWidget * context);
	void                         OnSliderbarChanged        (UIWidget * context);
	bool                         OnMessage                 (UIWidget * context, const UIMessage & msg);
	void                         OnGenericSelectionChanged (UIWidget * context);
	void                         OnCheckboxSet             (UIWidget * context);
	void                         OnCheckboxUnset           (UIWidget * context);
	void                         OnPopupMenuSelection      (UIWidget * context);

	void                         update                    (float deltaTimeSecs);

	UIPoint                      transformWorldToPage      (const Vector & vect)   const;
	Vector                       transformPageToZone3d     (const UIPoint & pt)    const;

	Vector2d                     transformPageToZone       (const UIPoint & pt)    const;
	UIPoint                      transformWorldToPage      (const Vector2d & vect) const;

	typedef std::pair <ClientObject *, ClientObject *> ContainerMsg;
	void                         onAddedRemovedContainer   (const ContainerMsg & msg);
	void                         onLocationsReceived       (const PlanetMapManagerClient::Messages::LocationsReceived::Payload &);
	void                         onLocationsRequested      (const bool &);

	void						 onGCWGroupValuesUpdated   (const GuildObject::Messages::GCWGroupScoreUpdatedThisGalaxy::Payload & obj);

	void                         saveSettings              () const;
	void                         loadSettings              ();

	void                         onObjectNameChange        (const ClientObject & obj);
	void                         onWaypointChange          (const ClientWaypointObject & waypoint);

private:

	SwgCuiPlanetMap (const SwgCuiPlanetMap & rhs);
	SwgCuiPlanetMap & operator= (const SwgCuiPlanetMap & rhs);

	void                         updateZoom            ();
	void                         computeCenterPoint    (const UIPoint & center);
	UIPoint                      getTargetPoint        () const;
	void                         reset                 ();
	void                         handleCheckbox        (const UICheckbox & box);
	void                         updateMarkers         ();
	void                         updateLabelVisibility () const;
	void                         setupMarkersForType   (uint8 category, uint8 subCategory, const NetworkId & networkId, bool activeWaypointsOnly, bool skipActiveWaypoints, std::string const locationName = "");
	UIPopupMenu *                createPopupForEntry   (const Vector2d & pos, const Unicode::String & entryName, const NetworkId & id);
	void                         setupCurrentZone      ();
	void                         clearMarkers          ();
	void                         updatePlayerIcon      ();
	void                         setZoneSupermapMode   (bool active);
	
	UIPage *                     m_pagePlanet;
	UISliderbar *                m_sliderZoom;

	UISize                       m_maxPlanetScrollExtent;

	UIPoint                      m_lastScrollLocation;
	UIFloatPoint                 m_centerPointTarget;
	UIFloatPoint                 m_centerPoint;
	bool                         m_interpolating;
	bool                         m_ignoreNextScroll;
	UIPoint                      m_centerPointOffset;

	UICheckbox *                 m_checkShowCities;
	UICheckbox *                 m_checkShowWaypoints;
	UICheckbox *                 m_checkShowNames;
	UICheckbox *				 m_checkShowGCWRegions;

	UITreeView *                 m_treeLocations;

	UIWidget *                   m_iconHere;

	UIFloatPoint                 m_zoneSize;

	UIButton *                   m_buttonSample;
	UIText *                     m_textSample;
	UIPage *                     m_pageMarkers;

	UIText *                     m_textPosition;
	UIText *                     m_textHeadingRange;

	UIColorEffector *            m_effectorWaypointFlash;

	typedef stdvector<UIButton *>::fwd ButtonVector;
	typedef stdvector<UIText *>::fwd   TextVector;

	ButtonVector *               m_freeListButton;
	TextVector *                 m_freeListText;

	UIText *                     m_textPlanetName;

	UIPage *                     m_pageMaps;
	MessageDispatch::Callback *  m_callback;

	UISize                       m_lastSize;

	std::string                  m_lastZone;
	std::string                  m_lastSelectedLocationPath;

	typedef stdmap<UIButton *, UIText *>::fwd ButtonTextMap;
	ButtonTextMap *              m_buttonTextMap;

	UIButton *                   m_buttonRefresh;

	typedef stdmap<uint8, UIButton *>::fwd SampleButtonMap;
	SampleButtonMap *            m_sampleButtonMap;

	Unicode::String              m_iconPathInactive;
	Unicode::String              m_iconPathActive;

	UIButton *                   m_buttonZoom;
	ZoomLevel                    m_zoomLevel;

	Vector m_zoneWorldCoordinateOffset;

	bool                         m_entriesRequested;
};

//======================================================================

#endif
