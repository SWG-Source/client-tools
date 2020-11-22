//======================================================================
//
// SwgCuiHudSpace.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiHudSpace_H
#define INCLUDED_SwgCuiHudSpace_H

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"
#include "swgClientUserInterface/SwgCuiHud.h"

//----------------------------------------------------------------------

class CreateMissileMessage;
class ShipDamageMessage;
class UIImage;

//----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//======================================================================

class SwgCuiHudSpace : public SwgCuiHud
{
public:

	static void createHudTemplate();
	static void createFreshHud();

	

public:
	                         SwgCuiHudSpace (UIPage & page);
	                         ~SwgCuiHudSpace ();

	virtual void update(float updateDeltaSeconds);
	virtual void performActivate();
	virtual void performDeactivate();

	void onShipDamaged(ShipDamageMessage const & shipDamage);
	void onCreateMissileMessage(const CreateMissileMessage & createMissileMessage);

	typedef std::pair<uint32, int> ChassisAndStationPair;
	typedef std::pair<std::string, bool> ShipPageAndStatusPair;

	static void setReticleVisible(bool b);

protected:

	virtual bool shouldRenderReticle() const;

private:
	                         SwgCuiHudSpace ();
	                         SwgCuiHudSpace (const SwgCuiHudSpace & rhs);
	SwgCuiHudSpace & operator= (const SwgCuiHudSpace & rhs);

	UIPage * m_damagePage;
	UIPage * m_nebulaHuePage;

	MessageDispatch::Callback * m_callback;
	stdmultimap<ChassisAndStationPair, ShipPageAndStatusPair>::fwd *m_spaceHudPageStates;

	UIButton * m_ejectButton;
	UIButton * m_exitStationButton;
	UIButton * m_tutorialButton;
	UIButton * m_enterSpaceButton;
	UIPage * m_buttonParentPage;
	UIPage * m_missileLockOnYouPage;
	Timer m_missileLockTimer;
	UIWidget * m_missileLockWidget;
	UIEffector * m_missileLockEffector;
};

//======================================================================

#endif
