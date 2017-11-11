//======================================================================
//
// SwgCuiCombatState.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiCombatState_H
#define INCLUDED_SwgCuiCombatState_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

//----------------------------------------------------------------------

class UIPage;
class UIButton;
class UIComposite;
class UIOpacityEffector;
class CreatureObject;

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiCombatState :
public CuiMediator,
public UIEventCallback
{
public:

	class Action;

	explicit            SwgCuiCombatState (UIPage & page);

	virtual void        OnButtonPressed      (UIWidget * context);

	void                updateAll ();

	void                requestPostureChange  (int posture);

	void                onPostureChanged (const CreatureObject & creature);

protected:
	virtual void        performActivate ();
	virtual void        performDeactivate ();

private:
	virtual            ~SwgCuiCombatState ();
	                    SwgCuiCombatState ();
	                    SwgCuiCombatState (const SwgCuiCombatState &);
	SwgCuiCombatState & operator= (const SwgCuiCombatState &);

	void                zeroArrays ();

	void                updatePosture  ();

	enum 
	{
		NUM_POSTURES = 4
	};


	UIButton *                   m_postureButton   [NUM_POSTURES];

	UIOpacityEffector *          m_effectorBgFadeFull;
	UIOpacityEffector *          m_effectorBgFadeOut;

	Action *                     m_action;

	uint8                        m_lastPosture;

	MessageDispatch::Callback *  m_callback;
};

//====================================================================

#endif
