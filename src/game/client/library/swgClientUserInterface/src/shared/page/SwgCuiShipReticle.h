//======================================================================
//
// SwgCuiShipReticle.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiShipReticle_H
#define INCLUDED_SwgCuiShipReticle_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "sharedUtility/CallbackReceiver.h"

#include "UIEventCallback.h"

//----------------------------------------------------------------------

class CuiArcGuage;
class UIImage;
class UIText;
class UIButton;
class Vector;

//----------------------------------------------------------------------

class SwgCuiShipReticle : 
public CuiMediator, public CallbackReceiver
{
public:
	explicit SwgCuiShipReticle(UIPage & page);

	void enableReticle(int index);
	void update(float deltaTimeSecs);

	void setReticleVisible(bool b);

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	SwgCuiShipReticle();
	~SwgCuiShipReticle();
	SwgCuiShipReticle(const SwgCuiShipReticle &);
	SwgCuiShipReticle & operator=(const SwgCuiShipReticle &);

	void performCallback();

private:
	typedef stdvector<UIImage *>::fwd ImageVector;

	CuiArcGuage * m_arcSpeed;
	CuiArcGuage * m_arcWeapon;
	CuiArcGuage * m_arcBooster;
	ImageVector * m_reticles;
	UIImage * m_reticleCurrent;
};

//======================================================================

#endif
