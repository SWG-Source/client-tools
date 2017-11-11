// ======================================================================
//
// SwgCuiSceneSelection.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiSceneSelection_H
#define INCLUDED_SwgCuiSceneSelection_H

// ======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIPage;
class UIListbox;
class UIButton;
class UIText;
class UIImage;
class UIData;
class CreatureObject;
class UITextbox;

//-----------------------------------------------------------------

class SwgCuiSceneSelection :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                 SwgCuiSceneSelection (UIPage & thePage);

	//- PS UI support
	virtual void             OnButtonPressed   (UIWidget *Context);
	virtual bool             OnMessage         (UIWidget *Context, const UIMessage & msg);
	virtual void             OnListboxSelectionChanged ( UIWidget *context );


	virtual void             performActivate   ();
	virtual void             performDeactivate ();

//	const UINarrowString &   getSelectedScenefile () const;
	bool                     startScene ( const UINarrowString & avatarName, CreatureObject * customizedPlayer) const;

private:
	virtual                 ~SwgCuiSceneSelection ();
	                         SwgCuiSceneSelection ();
	                         SwgCuiSceneSelection (const SwgCuiSceneSelection & rhs);
	SwgCuiSceneSelection &   operator=    (const SwgCuiSceneSelection & rhs);

	void                     updateSceneSelection ();

private:

	UIButton *               m_okButton;
	UIButton *               m_cancelButton;

	UIText *                 m_descriptionText;
	UIImage *                m_placeholderImage;

	UIListbox *              m_selectionListbox;

	UITextbox *              m_textbox;

	UIButton *               m_buttonCreate;

};

// ======================================================================

#endif
