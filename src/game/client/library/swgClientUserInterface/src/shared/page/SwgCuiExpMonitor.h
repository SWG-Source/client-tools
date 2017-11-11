//======================================================================
//
// SwgCuiExpMonitor.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiExpMonitor_H
#define INCLUDED_SwgCuiExpMonitor_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class CreatureObject;
class PlayerObject;
class UIImage;
class UIText;
class UIComboBox;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiExpMonitor :
public CuiMediator,
public UIEventCallback
{
public:

	                          SwgCuiExpMonitor              (UIPage & page);

	void                      onPlayerSkillsChanged    (const CreatureObject & creature);
	void                      onPlayerSetup            (const CreatureObject & creature);
	void                      onPlayerExpChanged       (const PlayerObject & creature);
	void                      onPlayerExpChanged2      (const CreatureObject & creature);
	void                      onWorkingSkillChanged(const std::string &);
    
	bool                      close                    ();

protected:

	virtual void             performActivate   ();
	virtual void             performDeactivate ();

private:

	void                     updateSkillText();
	void                     updateBar();
	void                     placeTicks();

	                        ~SwgCuiExpMonitor ();
	SwgCuiExpMonitor ();
	SwgCuiExpMonitor (const SwgCuiExpMonitor & rhs);
	SwgCuiExpMonitor & operator= (const SwgCuiExpMonitor & rhs);

	UIText *                    m_textSkill;
	UIPage *                    m_pageBar;
	UIImage *                   m_imageTick;

	MessageDispatch::Callback * m_callback;

	UIColor                     m_colorComplete;
	UIColor                     m_colorDefault;
};

//======================================================================

#endif
