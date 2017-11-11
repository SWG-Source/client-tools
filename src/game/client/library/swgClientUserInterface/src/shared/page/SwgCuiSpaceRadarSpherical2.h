//======================================================================
//
// SwgCuiSpaceRadarSpherical2.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiSpaceRadarSpherical2_H
#define INCLUDED_SwgCuiSpaceRadarSpherical2_H

//======================================================================

#include "UIEventCallback.h"
#include "clientUserInterface/CuiMediator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"

class UIButton;
class UIEffector;
class UIEllipse;
class UIImage;
class Vector;

//----------------------------------------------------------------------

class SwgCuiSpaceRadarSpherical2 : 
public CuiMediator, 
public UIEventCallback
{
public:
	class BlipPane;

	explicit SwgCuiSpaceRadarSpherical2(UIPage & page);

	void update(float deltaTimeSecs);

protected:
	virtual void performActivate();
	virtual void performDeactivate();

	typedef stdvector<UIEllipse*>::fwd EllipseVector;
	static void updateEllipseVector(EllipseVector *vec, float num, float den);

	void enableShieldEffector(bool const isEnabled);
	void updateShieldEffector();
	void updateSpeedGauge();
	void updateWeaponGauge();

private:
	virtual ~SwgCuiSpaceRadarSpherical2();
	SwgCuiSpaceRadarSpherical2();
	SwgCuiSpaceRadarSpherical2(const SwgCuiSpaceRadarSpherical2 &);
	SwgCuiSpaceRadarSpherical2 & operator=(const SwgCuiSpaceRadarSpherical2 &);

private:
	BlipPane * m_blipPane;
	UIPage * m_pageSquare;
	UIPoint m_lastCoord;
	UIPoint m_lastSize;

	EllipseVector * m_frontShields;
	EllipseVector * m_backShields;
	EllipseVector * m_frontArmor;
	EllipseVector * m_backArmor;
	
	UIEffector * m_shieldOverDriveEffector;
	UIPage * m_shieldPage;
	bool m_isShieldEffectorEnabled;

	UIEllipse * m_speedBarCurrent;
	UIEllipse * m_speedBarMax;
	UIEllipse * m_speedOverdrive;
	UIEllipse * m_speedBarBoost;

	UIEllipse * m_weaponCurrent;
};

//======================================================================

#endif
