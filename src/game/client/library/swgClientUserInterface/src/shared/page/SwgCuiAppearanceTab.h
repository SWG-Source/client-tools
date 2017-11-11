//======================================================================
//
// SwgCuiAppearanceTab.h
// copyright (c) 2008 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAppearanceTab_H
#define INCLUDED_SwgCuiAppearanceTab_H

//======================================================================
#include "clientUserInterface/CuiMediator.h"

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"

#include "UIEventCallback.h"
//----------------------------------------------------------------------

class CuiWidget3dObjectListViewer;
class UIPage;
class UIText;
class UIButton;
class UICheckbox;
class CuiDragInfo;

#include <vector>
//----------------------------------------------------------------------

class SwgCuiAppearanceTab: public CuiMediator, public UIEventCallback
{
public:
	explicit SwgCuiAppearanceTab(UIPage & page);
	~SwgCuiAppearanceTab();

	virtual void update(float delta);

	void         OnButtonPressed(UIWidget *context );
	void         OnCheckboxSet(UIWidget *context);
	void         OnCheckboxUnset(UIWidget *context);

	bool         OnMessage(UIWidget *Context, const UIMessage & msg );

	typedef stdvector<UIText *>::fwd                      TextVector;
	typedef stdvector<CuiWidget3dObjectListViewer *>::fwd ViewerVector;

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	//disabled
	SwgCuiAppearanceTab(const SwgCuiAppearanceTab & rhs);
	SwgCuiAppearanceTab & operator= (const SwgCuiAppearanceTab & rhs);

private:

	CuiWidget3dObjectListViewer * m_characterViewer;
	UIText*                       m_characterName;
	UIButton*                     m_closeButton;
	UICheckbox*					  m_showInventoryItems;
	UIPage*                       m_viewerPage;

	ViewerVector                  m_slotViewers;
	TextVector                    m_slotText;

	CuiDragInfo *                 m_lastDragItem;
	Timer                         m_dragTimer;
};

//======================================================================

#endif