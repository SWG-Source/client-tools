//======================================================================
//
// SwgCuiGroundRadar.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiGroundRadar_H
#define INCLUDED_SwgCuiGroundRadar_H

//======================================================================

#include "swgClientUserInterface/SwgCuiLockableMediator.h"
#include "StringId.h"

class ClientProceduralTerrainAppearance;
class UIImage;
class CuiWidgetGroundRadar;
class UIText;
class Vector;
class UIButton;

//----------------------------------------------------------------------

class SwgCuiGroundRadar : 
public SwgCuiLockableMediator, 
public UIWidget::TooltipCallback
{
public:
	class BlipPane;
	class WaypointPane;

	explicit            SwgCuiGroundRadar  (UIPage & page);

	void                updateRadar        (const Vector & pos, const ClientProceduralTerrainAppearance * cmtat, bool force);

	void                setAngle           (float theta, bool force);

	virtual bool        OnMessage          (UIWidget *context, const UIMessage & msg);
	virtual void        OnButtonPressed    (UIWidget *context);
	
	virtual void        OnCheckboxSet( UIWidget *Context );
	virtual void        OnCheckboxUnset( UIWidget *Context );
	virtual void        OnSizeChanged(UIWidget * context);
	virtual void        OnWidgetRectChanging (UIWidget * context, UIRect & targetRect);

	void                onSceneChanged     ();

	int                 getRadarRange      () const;

	void                zoomIn             ();
	void                zoomOut            ();
	void                setConMode         (bool conMode);

	bool                getConMode         () const;

	void                update             (float deltaTimeSecs);

	void                saveSettings       () const;
	void                loadSettings       ();

	virtual UIString const & getTooltip(UIPoint const & point);

	void setAlwaysShowRangeInRadar(bool alwaysShow);

protected:
	virtual void        performActivate    ();
	virtual void        performDeactivate  ();

private:
	virtual            ~SwgCuiGroundRadar  ();
	SwgCuiGroundRadar ();
	SwgCuiGroundRadar (const SwgCuiGroundRadar &);
	SwgCuiGroundRadar & operator= (const SwgCuiGroundRadar &);

	void positionWidgetAroundRadarWithRect(UIWidget *widget, double angle, UIRect & targetRect);

	UIImage *                    m_radarCompass;
	UIPage *                     m_radarCompassTop;
	UIImage *                    m_radarCompassTopSmall;
	UIImage *                    m_radarCompassTopMedium;
	UIImage *                    m_radarCompassTopLarge;
	CuiWidgetGroundRadar *       m_groundRadar;
	UIText *                     m_rangeIndicatorText;
	UIText *                     m_regionIndicatorText;
	float                        m_lastRadarUpdate;
	UIButton *                   m_topZoomButton;
	UIButton *                   m_bottomZoomButton;
	UIImage *                    m_iconImage;

	UIImage *                    m_blipImage;

	BlipPane *                   m_blipPane;
	WaypointPane *               m_waypointPane;

	int                          m_radarRange;
	float                        m_radarDirty;

	class SwgCuiGroundRadarAction;
	SwgCuiGroundRadarAction *    m_action;

	UIText *                     m_textLat;
	UIText *                     m_textHeight;
	UIText *                     m_textLong;

	UIPoint                      m_lastCoord;
	UIPage *                     m_pageSquare;
	
	UICheckbox*                  m_conModeCheckbox;
	bool                         m_conMode;

	StringId                     m_lastRegion;
	UIString                     m_radarTooltip;

	UISize                       m_previousSize;

	UIImage *                    m_radarSkinSmall;
	UIImage *                    m_radarSkinMedium;
	UIImage *                    m_radarSkinLarge;
	int                          m_radarSkinSmallWidth;
	int                          m_radarSkinMediumWidth;
	int                          m_radarSkinLargeWidth;

	int                          m_radarToParentOffset;


};

//-----------------------------------------------------------------

inline int SwgCuiGroundRadar::getRadarRange () const
{
	return m_radarRange;
}

//======================================================================

#endif
