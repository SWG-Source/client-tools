//===================================================================
//
// SwgCuiSpaceZoneMap.h
// copyright 2004, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_SwgCuiSpaceZoneMap_H
#define INCLUDED_SwgCuiSpaceZoneMap_H

//===================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"
#include "clientGame/ZoneMapObject.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedMath/Vector.h"
#include "sharedMessageDispatch/Receiver.h"

//===================================================================

class UIButton;
class UITreeView;
class CuiWidget3dObjectListViewer;
class PlayerObject;
class Object;
class CreatureObject;
class UIDataSourceContainer;
class ZoneMapObject;
class UISliderbar;
class UIText;
class UIWidget;

namespace MessageDispatch
{
	class Callback;
};

//===================================================================

class SwgCuiSpaceZoneMap
: public UIEventCallback
, public CuiMediator
, public MessageDispatch::Receiver
{
public:
	explicit SwgCuiSpaceZoneMap(UIPage& page);

	void setSceneId(std::string const & sceneId);

	virtual void update(float deltaTimeSecs);

	void receiveMessage(const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message);

	virtual void OnButtonPressed(UIWidget* context);
	virtual void OnCheckboxSet(UIWidget * context);
	virtual void OnCheckboxUnset(UIWidget * context);
	virtual void OnSliderbarChanged(UIWidget * context);
	virtual bool OnMessage(UIWidget * context, UIMessage const & msg);
	virtual void OnPopupMenuSelection(UIWidget * context);

	void onPlayerWaypointsChanged(PlayerObject const &);
	void onPlayerMissionCriticalsChanged(CreatureObject const &);

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	SwgCuiSpaceZoneMap();
	SwgCuiSpaceZoneMap(const SwgCuiSpaceZoneMap &);
	SwgCuiSpaceZoneMap & operator=(const SwgCuiSpaceZoneMap &);
	virtual ~SwgCuiSpaceZoneMap();

	UIDataSourceContainer * findItemFromZoneObjectData(ZoneMapObject::ObjectData const * objectData) const;
	ZoneMapObject::ObjectData const * findZoneObjectDataFromItem(UIDataSourceContainer const * item) const;
	void synchronizeSelection(ZoneMapObject::ObjectData const * objectData);
	void targetItem(UIDataSourceContainer const * item) const;
	void autopilotToItem(UIDataSourceContainer const * item) const;

	Vector const & calculateLookAtTarget();
	float getParametricZoomTime() const;
	void setParametricZoomTime(float const time);
	float calculateZoom() const;
	void moveCamera(Vector const & direction);
	void createContextMenu(ZoneMapObject::ObjectData const * pickedObject, UIDataSourceContainer const * pickedItem);

	void pickTargetToLookAt(Object * target, bool const setFocusTo);
	void resetView();

	void populateZoneMap(bool const reset);
	void populateSpaceStationTree();
	void populateHyperspacePointTree();
	void populateActiveWaypointTree();
	void populateInactiveWaypointTree();
	void populateActivePoiWaypointTree();
	void populateInactivePoiWaypointTree();
	void populateMissionCriticalTree();
	void populateNamesInViewer();

private:
	UIButton * m_exit;
	UICheckbox * m_showAsteroids;
	UICheckbox * m_showNebulas;
	UICheckbox * m_showHyperspacePoint;
	UICheckbox * m_showSpaceStations;
	UICheckbox * m_showWaypoints;
	UICheckbox * m_showMissionCritical;
	UICheckbox * m_showPlayerShip;
	UICheckbox * m_showNames;
	UITreeView * m_detailTree;
	UIText * m_zoneName;
	UISliderbar * m_zoomSlider;
	UIButton * m_resetView;

	CuiWidget3dObjectListViewer * m_viewer;
	UICursor * m_dragCursor;
	UICursor * m_rotateCursor;
	UICursor * m_selectCursor;
	UICursor * m_zoomInCursor;
	UICursor * m_zoomOutCursor;

	MessageDispatch::Callback * m_callback;

	ZoneMapObject * m_zoneMapObject;

	std::string m_sceneId;
	bool m_refreshZoneMap;

	bool m_modifyWaypoints;
	bool m_modifyMissionCriticals;
	bool m_displayingCurrentZone;

	float m_elapsedTime;
	Vector m_lookAtTarget;
	Vector m_previousLookAtTarget;
	float m_lerpPreviousLookAtTargetToLookAtTargetSeconds;
	float m_parametricZoomTime;

	UIPoint m_lastMousePoint;

	ZoneMapObject::TextAndLocation m_highlightedObjectNameAndLocation;
	bool m_textAndLocationsChanged;
	int m_numberOfTextAndLocations;
};

//===================================================================

#endif
