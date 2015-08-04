//======================================================================
//
// SwgCuiKillMeter.h
// copyright (c) 2001 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiKillMeter_H
#define INCLUDED_SwgCuiKillMeter_H

//======================================================================

#include "swgClientUserInterface/SwgCuiLockableMediator.h"
#include "StringId.h"

class CreatureObject;
class UIText;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiKillMeter : 
public SwgCuiLockableMediator 
{
public:

	explicit            SwgCuiKillMeter  (UIPage & page);

	void                update             (float deltaTimeSecs);

	void                onSkillTemplateChanged  (std::string const & newTemplate);

protected:
	virtual void        performActivate    ();
	virtual void        performDeactivate  ();

private:
	virtual            ~SwgCuiKillMeter  ();
	SwgCuiKillMeter ();
	SwgCuiKillMeter (const SwgCuiKillMeter &);
	SwgCuiKillMeter & operator= (const SwgCuiKillMeter &);

	bool shouldShowKillMeter();

	MessageDispatch::Callback *	 m_callback;
	int                          m_lastValue;
	UIText *                     m_textKills;

	bool                         m_waitingForSkills;
};

//======================================================================

#endif
