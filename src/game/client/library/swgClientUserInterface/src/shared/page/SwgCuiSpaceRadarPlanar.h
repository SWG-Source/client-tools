//======================================================================
//
// SwgCuiSpaceRadarPlanar.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiSpaceRadarPlanar_H
#define INCLUDED_SwgCuiSpaceRadarPlanar_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"

#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

#include "UIEventCallback.h"

//----------------------------------------------------------------------

class ClientObject;
class UIButton;
class UIEllipse;
class UIGBuffer;
class UIWidget;
class Vector;

//----------------------------------------------------------------------

class SwgCuiSpaceRadarPlanar :
public CuiMediator,
public UIEventCallback
{
public:
	class Blip;

	explicit SwgCuiSpaceRadarPlanar(UIPage & page);

	virtual void update(float deltaTimeSecs);

	UIWidget * newHeadImage();
	UIWidget * newTailImage();
	void deleteWidget(UIWidget * widget);

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	virtual ~SwgCuiSpaceRadarPlanar();
	SwgCuiSpaceRadarPlanar();
	SwgCuiSpaceRadarPlanar(const SwgCuiSpaceRadarPlanar &);
	SwgCuiSpaceRadarPlanar & operator=(SwgCuiSpaceRadarPlanar const &);

	Blip * getBlipReference(NetworkId const & netId);
	void blipRelinquish(Blip *);
	void blipRelinquishById(NetworkId const & objectId);
	Blip * blipAcquire();
	void updateBlipFor(ClientObject const * obj, Transform const & playerXform_p2w);
	void decayBlip(float timeDelta, Vector const & playerPos);

	void getViewOffset(Vector const &input, UIPoint & output) const;

	void updateViewInfo();
	UIWidget * duplicate(UIWidget const * source);

private:
	Transform m_transformView;
	Vector m_viewOffset;
	Vector m_viewScale;

	typedef stdset<Blip *>::fwd BlipCache;
	BlipCache * m_blipCache;

	typedef stdmap<NetworkId /*object*/, Blip * /*the blip info*/>::fwd BlipMap;
	BlipMap * m_blipMap;

	UIWidget * m_headImage;
	UIWidget * m_tailImage;

	UIGBuffer * m_gbuffer;
};

//======================================================================

#endif
