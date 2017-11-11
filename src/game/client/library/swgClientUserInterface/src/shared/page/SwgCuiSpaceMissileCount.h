//=============================================================================
//
// SwgCuiSpaceMissileCount.h
// copyright (c) 2004 Sony Online Entertainment
//
//=============================================================================

#ifndef INCLUDED_SwgCuiSpaceMissileCount_H
#define INCLUDED_SwgCuiSpaceMissileCount_H

//=============================================================================

#include "swgClientUserInterface/SwgCuiLockableMediator.h"
//-----------------------------------------------------------------------------

class UIText;

//-----------------------------------------------------------------------------

class SwgCuiSpaceMissileCount :
public SwgCuiLockableMediator
{
public:
	SwgCuiSpaceMissileCount(UIPage & page);
	virtual void update(float updateDeltaSeconds);

protected:
	virtual ~SwgCuiSpaceMissileCount();

private:
	SwgCuiSpaceMissileCount();
	SwgCuiSpaceMissileCount(const SwgCuiSpaceMissileCount &);
	SwgCuiSpaceMissileCount & operator=(const SwgCuiSpaceMissileCount &);

private:

	static int const c_maxNumMissileSlots = 4;
	static int const c_maxNumCounterMeasureSlots = 2;
	UIText* m_missileNames[c_maxNumMissileSlots];
	UIText* m_missileValues[c_maxNumMissileSlots];
	UIText* m_counterMeasureNames[c_maxNumCounterMeasureSlots];
	UIText* m_counterMeasureValues[c_maxNumCounterMeasureSlots];
};

//=============================================================================

#endif
