//======================================================================
//
// SwgCuiSpaceRadarSpherical_BlipPane.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiSpaceRadarSpherical_BlipPane_H
#define INCLUDED_SwgCuiSpaceRadarSpherical_BlipPane_H

//======================================================================

#include "UIWidget.h"
#include "swgClientUserInterface/SwgCuiSpaceRadarSpherical.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"

class ClientObject;
class Object;
class ShipDamageMessage;
class ShipObject;
class UIImage;
class VectorArgb;

//----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiSpaceRadarSpherical::BlipPane :
public UIWidget
{
public:
	typedef stdmap<const NetworkId /*attacker*/, float /*timeout*/>::fwd DamagerMap;

	typedef stdvector<UILine>::fwd LineSegmentVector;


	                       BlipPane          (UIImage & blipImage, UIImage & blipImageCorpse, UIImage & blipImageWaypoint, UIImage & blipImageStructure, UIImage & bracket, UIImage & blipImageGroup);

	virtual UIBaseObject * Clone             () const { return 0; }
	virtual UIStyle *      GetStyle          () const { return 0; }

	void                   renderWorldObject (ShipObject const * const ship, UICanvas & canvas, const ClientObject & object, const NetworkId & lookAtTargetGroupId) const;

	void                   Render            (UICanvas & canvas) const;
	void                   renderBlip        (ShipObject const * const ship, UICanvas & canvas, Vector const & worldPos, float const worldRadius, UIImage & blip, const UIColor & color, bool const allowWaypointArrow, bool const isWaypoint, bool const isBracket, bool const isTargetGroup, bool const isBlinking, bool const isCapitalShip) const;

	bool                   convertToWidgetSpace(ShipObject const * const ship, Vector &widgetPosition, bool abortOutOfRange, float & distanceToObject) const;

	void                   onShipDamaged     (const ShipDamageMessage &);

	void                   update            (float deltaSeconds);

private:
	                      ~BlipPane();
	                       BlipPane          ();
	                       BlipPane          (const BlipPane & rhs);
	BlipPane &             operator=         (const BlipPane & rhs);

	void                   damagerAdd        (const NetworkId & blipNetworkId);
	bool                   isDamager         (const NetworkId & blipNetworkId) const;

	UIImage &                    m_blipImage;
	UIImage &                    m_blipImageCorpse;
	UIImage &                    m_blipImageWaypoint;
	UIImage &                    m_blipImageStructure;
	UIImage &                    m_blipImageGroup;
	UIImage &                    m_blipImageTargetBracket;
	MessageDispatch::Callback *  m_callback;
	DamagerMap *                 m_damagers;
	Timer                        m_blinkTimer;
	bool                         m_blinkEnabled;
};

//======================================================================

#endif
