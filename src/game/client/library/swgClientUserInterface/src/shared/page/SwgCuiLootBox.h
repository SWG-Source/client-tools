//======================================================================
//
// SwgCuiLootBox.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiLootBox_H
#define INCLUDED_SwgCuiLootBox_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

//----------------------------------------------------------------------

class UIButton;
class UIComposite;
class UIPage;

//----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//----------------------------------------------------------------------

class SwgCuiLootBox : public CuiMediator, public UIEventCallback
{
public:
	SwgCuiLootBox                             (UIPage & page);
	~SwgCuiLootBox                            ();
	virtual void             OnButtonPressed  (UIWidget * context);
	void                     clearRewards     ();
	void                     addReward        (NetworkId const & objectNid);

protected:
	virtual void             performActivate  ();
	virtual void             performDeactivate();

private:
	//disabled
	                         SwgCuiLootBox  ();
	                         SwgCuiLootBox  (const SwgCuiLootBox & rhs);
	SwgCuiLootBox &          operator=      (const SwgCuiLootBox & rhs);

private:
	MessageDispatch::Callback * m_callback;
	UIButton *                  m_okButton;
	UIComposite *               m_comp;
	UIComposite *               m_sampleItem;
	UIPage *                    m_endSpacer;
};

// ======================================================================

#endif

