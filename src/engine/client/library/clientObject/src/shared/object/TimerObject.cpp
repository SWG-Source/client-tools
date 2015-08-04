// ======================================================================
//
// TimerObject.cpp
// asommers 8-2-99
//
// copyright 1999, bootprint entertainment
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "clientObject/FirstClientObject.h"
#include "clientObject/TimerObject.h"

#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedObject/AlterResult.h"

// ======================================================================
// STATIC PUBLIC TrailObject
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL (TimerObject, true, 0, 0, 0);

// ======================================================================
// PUBLIC TimerObject
// ======================================================================

TimerObject::TimerObject (const float expireTime, bool ignoreFirstAlter) : 
	Object (),
	m_ignoreFirstAlter (ignoreFirstAlter),
	m_timer (expireTime)
{
}

//-------------------------------------------------------------------

TimerObject::~TimerObject ()
{
}

//-------------------------------------------------------------------

float TimerObject::alter (float elapsedTime)
{
	//-- skip first alter if instructed
	if (m_ignoreFirstAlter)
	{
		m_ignoreFirstAlter = false;
		return AlterResult::cms_alterNextFrame;
	}

	//-- figure out if we've run out of time
	float alterResult;

	if (m_timer.updateZero (elapsedTime))
		alterResult = AlterResult::cms_kill;
	else
	{
		alterResult = AlterResult::cms_alterNextFrame;

		//-- only alter base class if we haven't run out of time and we're not skipping the frame
		AlterResult::incorporateAlterResult(alterResult, Object::alter(elapsedTime));
	}

	//-- return results
	return alterResult;
}

// ======================================================================

