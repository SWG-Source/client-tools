//======================================================================
//
// SwgCuiAllTargetsSpace_DamagerData.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAllTargetsSpace_DamagerData_H
#define INCLUDED_SwgCuiAllTargetsSpace_DamagerData_H

//======================================================================

#include "swgClientUserInterface/SwgCuiAllTargetsSpace.h"
#include "sharedMath/Vector.h"

class UIPage;

//----------------------------------------------------------------------

class SwgCuiAllTargetsSpace::DamagerData
{
public:
	explicit DamagerData();
	DamagerData(float decayTime, UIPage * pageObject, Vector const & position);
	~DamagerData();

	float getTimer() const;
	void setTimer(float timer);
	
	void incrementTimer(float value);

	UIPage const * const getPage() const;
	UIPage * getPage();

	Vector const & getPosition() const;
	void setPosition(Vector const & postion);
private:
	DamagerData(DamagerData const & );

private:
	float m_damageTimer;
	UIPage * m_damagePage;
	Vector m_damagePosition;
};

//======================================================================

#endif
