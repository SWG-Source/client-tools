//======================================================================
//
// SwgCuiAllTargetsSpace.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAllTargetsSpace_H
#define INCLUDED_SwgCuiAllTargetsSpace_H

//======================================================================

#include "swgClientUserInterface/SwgCuiAllTargets.h"
#include <vector>

//----------------------------------------------------------------------

class ShipDamageMessage;
class UIImage;

//----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiAllTargetsSpace :
public SwgCuiAllTargets
{
public:
	class DamagerData;

	SwgCuiAllTargetsSpace(UIPage & page);

	void onShipDamaged(ShipDamageMessage const & shipDamage);
	virtual void updateOnRender();

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	~SwgCuiAllTargetsSpace();
	SwgCuiAllTargetsSpace();
	SwgCuiAllTargetsSpace(const SwgCuiAllTargetsSpace &);
	SwgCuiAllTargetsSpace & operator=(const SwgCuiAllTargetsSpace &);

	virtual void update(float deltaTimeSecs);

	void updateShipDamageIndicator(Camera const & camera);

	void updateTargetArrow(Camera const & camera);
	void updateShipTarget(Camera const & camera);
	void updateWaypointIndicators(Camera const& camera);
	void updateTurretTargets(Camera const& camera);
	bool isShipDamager(NetworkId const & networkId) const;
	UIPage * getShipArrowPage();

	void joystickImageConstruct();
	void joystickImageRelease();
	void joystickImageHide(bool hideEm);
	void joystickImageUpdate(UIPoint const & uiPosition, UIPoint const & centerPosition);

	virtual bool isWaypointVisible (Vector & effectiveWaypointPosition_o, Camera const & camera, Appearance const * appearance) const;

private:
	typedef stdmap<NetworkId, DamagerData*>::fwd DamagerMap;
	typedef stdstack<UIPage *>::fwd PageStack;
	typedef stdvector<UIImage *>::fwd ImageVector;

	MessageDispatch::Callback * m_callback;
	DamagerMap * m_damagers;
	PageStack * m_arrowPages;
	UIPage * m_damageDirectionArrow;

	UIImage * m_virtualJoystickImage;
	ImageVector * m_virtualJoystickImages;
	bool m_virtualJoystickReset;
	float m_timeDelta;

	UIImage * m_targetLeadIndicator;
	UIImage * m_targetLeadIndicatorOutOfRange;

	UIPage * m_sampleWaypointIndicatorPage;

	UIPage * m_containerPage;

	UIPage * m_targetArrowPage;

	typedef std::vector<UIPage *> PageVector;
	PageVector m_waypointIndicatorPages;
	PageVector m_turretTargetPages;

	UIPage * m_sampleTurretTargetArrow;
};

//======================================================================

#endif
