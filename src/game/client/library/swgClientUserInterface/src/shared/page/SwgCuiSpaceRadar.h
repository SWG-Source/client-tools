//======================================================================
//
// SwgCuiSpaceRadar.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiSpaceRadar_H
#define INCLUDED_SwgCuiSpaceRadar_H

//======================================================================

#include "UIEventCallback.h"
#include "clientGame/ShipStation.h"
#include "swgClientUserInterface/SwgCuiLockableMediator.h"
#include "sharedUtility/CallbackReceiver.h"

//----------------------------------------------------------------------

class ClientProceduralTerrainAppearance;
class SwgCuiSpaceRadarSpherical;
class UIButton;
class UIImage;
class UIText;
class Vector;


//----------------------------------------------------------------------

class SwgCuiSpaceRadar : 
public SwgCuiLockableMediator, 
public CallbackReceiver
{
public:
	explicit SwgCuiSpaceRadar(UIPage & page);

	void update(float deltaTimeSecs);
	void updateRadar(const Vector & pos, bool force);

	void zoomIn();
	void zoomOut();

	void saveSettings() const;
	void loadSettings();

	virtual void OnButtonPressed(UIWidget *context);
	virtual bool OnMessage(UIWidget *context, const UIMessage & msg);

	void setRadarActiveIndex(int index);
	int getRadarActiveIndex() const;
	CuiMediator * getRadarActiveMediator();

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	virtual ~SwgCuiSpaceRadar();
	SwgCuiSpaceRadar();
	SwgCuiSpaceRadar(const SwgCuiSpaceRadar &);
	SwgCuiSpaceRadar & operator=(const SwgCuiSpaceRadar &);

	void buildRadarPages();
	void releaseRadarPages();

	void performCallback();

	void initializeStations();
	void setStationIcon(ShipStation::Type station);

private:
	float m_lastRadarUpdate;
	bool m_radarDirty;
	
	UIImage * m_iconImage;
	UIPoint m_lastCoord;
	UIPoint m_lastSize;
	UIButton * m_topZoomButton;
	UIButton * m_bottomZoomButton;
	UIText * m_textLat;
	UIText * m_textHeight;
	UIText * m_textLong;
	UIText * m_rangeIndicatorText;
	
	class SwgCuiSpaceRadarAction;
	SwgCuiSpaceRadarAction * m_action;

	typedef stdvector<CuiMediator *>::fwd CuiMediatorVector;
	CuiMediatorVector * const m_radarMediators;
	int m_radarActiveIndex;

	typedef stdunordered_map<int, UIImage *>::fwd StationToImageMap;
	StationToImageMap * m_stationImageMap;
	UIImage * m_stationImageCurrent;
};

//======================================================================

#endif
