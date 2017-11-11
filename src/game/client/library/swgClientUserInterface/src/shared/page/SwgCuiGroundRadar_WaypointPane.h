//======================================================================
//
// SwgCuiGroundRadar_WaypointPane.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiGroundRadar_WaypointPane_H
#define INCLUDED_SwgCuiGroundRadar_WaypointPane_H

//======================================================================

#include "swgClientUserInterface/SwgCuiGroundRadar.h"
#include "UIWidget.h"

class UIImage;
class VectorArgb;
class Object;

//----------------------------------------------------------------------

class SwgCuiGroundRadar::WaypointPane :
public UIWidget
{
public:
	WaypointPane (const CuiWidgetGroundRadar & groundRadar, UIImage & arrow, UIImage & arrowWaypoint, UIImage & arrowGroup, UIImage & bracket);

	virtual UIBaseObject * Clone () const { return 0; }
	virtual UIStyle *      GetStyle () const { return 0; }

	void Render (UICanvas & canvas) const;

	struct Info
	{
		UIColor  color;
		float    angle;
		float    distance;
		bool     isWaypoint;
		bool     isBracket;
		bool     isGroup;
		Info (const UIColor & theColor, float theAngle, float theDistance, bool _isWaypoint, bool _isBracket, bool _isGroup) : color (theColor), angle (theAngle), distance (theDistance), isWaypoint (_isWaypoint), isBracket (_isBracket), isGroup (_isGroup) {}
	};

	void         clearWaypoints ();
	void         addWaypoint (const Info & info);

private:
	~WaypointPane ();
	WaypointPane ();
	WaypointPane (const WaypointPane & rhs);
	WaypointPane & operator= (const WaypointPane & rhs);

	const CuiWidgetGroundRadar & m_groundRadar;
	UIImage *                    m_arrow;
	UIImage *                    m_arrowWaypoint;
	UIImage *                    m_arrowGroup;
	UIImage *                    m_bracket;

	typedef stdvector<Info>::fwd InfoVector;
	InfoVector *                 m_infoVector;
};

//======================================================================

#endif
