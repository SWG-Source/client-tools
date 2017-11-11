//======================================================================
//
// SwgCuiButtonBar.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiButtonBar_H
#define INCLUDED_SwgCuiButtonBar_H

//======================================================================

#include "swgClientUserInterface/SwgCuiLockableMediator.h"

//----------------------------------------------------------------------

class PlayerObject;
class UIButton;
class UIComposite;
class UIEffector;
struct UIMessage;

//----------------------------------------------------------------------

namespace SwgCuiButtonBarNamespace
{
	class MyOpacityCallback;
}

class SwgCuiButtonBar :
public SwgCuiLockableMediator
{
public:

	explicit            SwgCuiButtonBar              (UIPage & page);

	bool                OnMessage (UIWidget * context, const UIMessage & msg);
	void                OnPopupMenuSelection (UIWidget * context);
	void                OnButtonPressed   (UIWidget * context);	
	virtual void        OnHoverIn(UIWidget * Context);
	virtual void        OnHoverOut(UIWidget * Context);

	void                ensureMenuIsVisible();
	void                toggleMenu();
	bool                isCompositeVisible();
	void                onOpacityCallback();
protected:

	void                performActivate              ();
	void                performDeactivate            ();
	void                update                       (float deltaTimeSecs);

private:
	                   ~SwgCuiButtonBar              ();
	                    SwgCuiButtonBar              ();
	                    SwgCuiButtonBar              (const SwgCuiButtonBar &);
	SwgCuiButtonBar &   operator=                    (const SwgCuiButtonBar &);

	
	void                turnOffInventoryEffector     ();
	void enableJournalEffector(bool isEnabled);
	void updateJournalEffector();	
	void updateExpertiseEffector();
	void updateMenuPosition();
	void updateMenuEffector();
	void updateMenuHighlight();

	void updateSkinnedImageState();

	
private:

	enum { MaxButtonStates = 4 };

	UIButton *           m_communityButton;
	UIButton *           m_mailButton;
	UIButton *           m_inventoryButton;
	UIButton *           m_journalButton;
	UIButton *           m_roadmapButton;
	UIButton *           m_mapButton;
	UIButton *           m_datapadButton;
	UIButton *           m_characterButton;
	UIButton *           m_expertiseButton;
	UIButton *           m_optionsButton;
	UIButton *           m_commandsButton;
	UIButton *           m_serviceButton;
	UIButton *           m_submenuButton;
	UIButton *           m_shipDetailsButton;
	UIButton *           m_homePortButton;
	UIButton *           m_myCollectionsButton;
	UIButton *           m_tcgButton;
	UIButton *           m_appearanceButton;
	UIButton *			 m_questBuilderButton;
	UIButton *           m_gcwInfoButton;

	UIEffector *         m_effectorNewMail;
	UIEffector *         m_effectorInventoryFull;
	UIEffector *         m_effectorMenu;
	UIEffector *         m_effectorExpertise;
	UIEffector *         m_journalButtonEffector;
	

	bool                 m_effectingNewMail;
	bool                 m_effectingInventoryFull;
	bool                 m_effectingMenu;
	bool                 m_effectingExpertise;
	bool                 m_journalMissionUpdate;
	int                  m_journalMissionCount;

	UIButton *           m_menuButton;
	UIComposite *        m_buttonsComposite;

	UIPage *             m_menuButtonPage;
	UIPage *             m_mouseoverPage;
	int                  m_numberButtons;

	UIWidget * m_buttonImages[MaxButtonStates];
	bool m_hoverState;

	UIPoint              m_menuButtonRestLoc;
	bool                 m_menuMovedIgnoredPress;
	bool                 m_scheduleButtonPress;

	SwgCuiButtonBarNamespace::MyOpacityCallback *m_opacityCallback;
};

//======================================================================

#endif
