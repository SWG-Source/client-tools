//======================================================================
//
// SwgCuiHudWindowManagerSpace.h
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiHudWindowManagerSpace_H
#define INCLUDED_SwgCuiHudWindowManagerSpace_H

#include "swgClientUserInterface/SwgCuiHudWindowManager.h"

class SwgCuiSpaceFlyOutPage;
class SwgCuiSpaceMissileCount;
class SwgCuiSpaceRadar;
class SwgCuiShipReticle;

//======================================================================

class SwgCuiHudWindowManagerSpace : public SwgCuiHudWindowManager
{
public:
	SwgCuiHudWindowManagerSpace(SwgCuiHud const & hud, CuiWorkspace & workspace);
	~SwgCuiHudWindowManagerSpace();

	virtual void receiveMessage(MessageDispatch::Emitter const & emitter, MessageDispatch::MessageBase const & message);

	void setReticleVisible(bool b);

#ifdef ENABLE_FORMATIONS
	void spawnSpaceAssignPlayerFormation() const;
#endif
	void spawnShipComponentDetail(NetworkId const & shipId) const;
	void spawnHyperspaceMap() const;

private:
	SwgCuiHudWindowManagerSpace();
	SwgCuiHudWindowManagerSpace(SwgCuiHudWindowManagerSpace const & rhs);
	SwgCuiHudWindowManagerSpace & operator=(SwgCuiHudWindowManagerSpace const & rhs);

private:

	SwgCuiSpaceRadar * m_spaceRadarMediator;
	SwgCuiSpaceFlyOutPage * m_spaceFlyOutPage;
	SwgCuiShipReticle * m_shipReticle;
	SwgCuiSpaceMissileCount * m_spaceMissileCount;
	SwgCuiTargets * m_targetsMediator;
};

//======================================================================

#endif
