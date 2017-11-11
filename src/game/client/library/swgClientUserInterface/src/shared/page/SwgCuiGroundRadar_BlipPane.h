//======================================================================
//
// SwgCuiGroundRadar_BlipPane.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiGroundRadar_BlipPane_H
#define INCLUDED_SwgCuiGroundRadar_BlipPane_H

//======================================================================

#include "swgClientUserInterface/SwgCuiGroundRadar.h"
#include "UIWidget.h"

class UIImage;
class VectorArgb;
class Object;
class ClientObject;

//----------------------------------------------------------------------

class SwgCuiGroundRadar::BlipPane :
public UIWidget
{
public:


	typedef stdvector<UILine>::fwd LineSegmentVector;


	                       BlipPane          (const CuiWidgetGroundRadar & groundRadar, UIImage & blipImage, UIImage & blipImageCorpse, UIImage & blipImageWaypoint, UIImage & blipImageStructure, UIImage & bracket, UIImage & blipImageGroup, UIImage & blipImageRing, UIImage & blipImageEntrance, WaypointPane & waypointPane, bool conMode);

	virtual UIBaseObject * Clone             () const { return 0; }
	virtual UIStyle *      GetStyle          () const { return 0; }

	void                   renderWorldObject (UICanvas & canvas, const ClientObject & object, const NetworkId & lookAtTargetGroupId) const;

	void                   Render            (UICanvas & canvas) const;
	void                   renderBlip        (ClientObject const & object, UICanvas & canvas, const Vector & worldPpos, const float worldRadius, UIImage & blip, const UIColor & color, bool allowWaypointArrow, bool isWaypoint, bool isBracket, bool isTargetGroup, bool isRing) const;


	void                   renderLineSegments (const UIPoint & objectPos, UICanvas & canvas, const LineSegmentVector & lsv, const UIColor & color) const;

	bool                   convertToWidgetSpace (const Vector & radarCenter, float & x, float & y, bool abortOutOfRange) const;

	void                   setConMode        (bool conMode);
	const bool             getConMode        () const;
	void resetSearchCache() const;

	ClientObject const * getToolTipObjectAtPoint(UIPoint const & point, UIColor & color) const;

private:
	                      ~BlipPane          ();
	                       BlipPane          ();
	                       BlipPane          (const BlipPane & rhs);
	BlipPane &             operator=         (const BlipPane & rhs);

	const CuiWidgetGroundRadar & m_groundRadar;

	UIImage &                    m_blipImage;
	UIImage &                    m_blipImageCorpse;
	UIImage &                    m_blipImageWaypoint;
	UIImage &                    m_blipImageStructure;
	UIImage &                    m_blipImageGroup;
	UIImage &                    m_blipImageRing;
	UIImage &                    m_blipImageTargetBracket;
	UIImage &					 m_blipImageEntrance;
	WaypointPane &               m_waypointPane;

	mutable float                m_cosAngle;
	mutable float                m_sinAngle;

	bool                         m_conMode;

	UIPoint                      m_lastCoord;
};

//======================================================================

#endif
