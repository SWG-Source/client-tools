// ======================================================================
//
// SwgCuiAvatarSetupProf.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiAvatarSetupProf_H
#define INCLUDED_SwgCuiAvatarSetupProf_H

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"
#include "sharedMath/Vector.h"

class UIPage;
class UIButton;
class UIComboBox;
class UIText;
class UITable;
class UIList;
class CuiWidget3dObjectListViewer;
class CreatureObject;
class CuiTurntableAdapter;

namespace MessageDispatch
{
	class Callback;
}

// ======================================================================

class SwgCuiAvatarSetupProf :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                 SwgCuiAvatarSetupProf (UIPage & page);

	virtual void             OnButtonPressed   (UIWidget *context);
	virtual bool             OnMessage( UIWidget *context, const UIMessage & msg );
	virtual void             OnShow(UIWidget *context);
	virtual void             OnGenericSelectionChanged (UIWidget * context);

	virtual void             performActivate();
	virtual void             performDeactivate();

	void                     onCreationFinished(bool b);
	void                     onCreationAborted(bool b);
	
	void                     update(float deltaTimeSecs);

private:
	virtual                 ~SwgCuiAvatarSetupProf ();
	                         SwgCuiAvatarSetupProf ();
	                         SwgCuiAvatarSetupProf (const SwgCuiAvatarSetupProf & rhs);
	SwgCuiAvatarSetupProf &  operator=             (const SwgCuiAvatarSetupProf & rhs);

	void                     updateProfessionInfo (bool playSound);
	void finishAndCreateCharacter();
	void returnToPreviousScreen(bool const dueToError);

private:

	UIButton *               m_buttonBack;
	UIButton *               m_buttonNext;

	UIComboBox *             m_combo;

	UIPage *                 m_pageDescription;
	UIText *                 m_textDescription;

	UIPage *                 m_pageSkills;
	UIText *                 m_textSkills;
	UIList *                 m_listSkills;

	UIPage *                 m_pageAttributes;
	UIText *                 m_textAttributes;
	UITable *                m_tableAttributes;

	CuiWidget3dObjectListViewer * m_viewer;

	int                           m_selectedIndex;

	typedef Watcher<CreatureObject> CreatureObjectWatcher;
	typedef stdvector<CreatureObjectWatcher>::fwd CreatureVector;
	CreatureVector *              m_creatures;

	CuiTurntableAdapter *         m_turntable;

	long                          m_lastSelectedAttribute;

	bool                          m_transitionFinished;

	bool					m_begunWookieeTransition;	

	MessageDispatch::Callback * m_callback;
	bool m_pendingCreationFinished;
	bool m_aborted;
};

// ======================================================================

#endif
