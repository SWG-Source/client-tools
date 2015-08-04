//======================================================================
//
// CuiShipInfo.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiShipInfo_H
#define INCLUDED_CuiShipInfo_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"

class CreatureObject;
class UIComposite;
class UIText;
class ShipObject;

//----------------------------------------------------------------------

class CuiShipInfo :
public CuiMediator
{
public:

	CuiShipInfo (const char * mediatorDebugName, UIPage & newPage);
	virtual void update (float timeElapsedSecs) = 0;

	void setHitPoints(float currentHP, float maxHP);
	void setArmorFront(float currentArmor, float maxArmor);
	void setArmorBack(float currentArmor, float maxArmor);
	void setShieldFront(float currentShield, float maxShield);
	void setShieldBack(float currentShield, float maxShield);
	void setTitle(const char * dialogTitle, const char * pilot);
	void setCapacitor(const float currentCapacitor, const float maxCapacitor);
	bool setAmmo(ShipObject const & shipObject);
	bool setRefireTimer(ShipObject const & shipObject);
	void setMissileTargetAcquired(ShipObject const & shipObject);

	void setVisible(bool isVisible);

	void setCommonShipInfo(ShipObject const * shipObject);

protected:

	virtual void performActivate ();
	virtual void performDeactivate ();
	virtual ~CuiShipInfo ();
	
	enum { TextBufferLength = 1024 };

	UIComposite *    m_comp;

private:

	CuiShipInfo ();
	CuiShipInfo (const CuiShipInfo& rhs);
	CuiShipInfo& operator= (const CuiShipInfo& rhs);

private:

	UIText *         m_textHitpoints;
	UIText *         m_textArmorFront;
	UIText *         m_textArmorBack;
	UIText *         m_textShieldFront;
	UIText *         m_textShieldBack;
	UIText *         m_textTitle;
	UIText *         m_textCapacitor;
	UIText *         m_textAmmo;
	UIText *         m_textRefire;
	UIText *         m_textTargetAcquired;

	int m_lastTextRefireLength;
	int m_lastTextAmmoLength;

	static char ms_textBuffer[ TextBufferLength ];
};

//======================================================================

#endif
