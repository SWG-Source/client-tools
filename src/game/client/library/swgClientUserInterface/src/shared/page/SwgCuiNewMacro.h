//======================================================================
//
// SwgCuiNewMacro.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiNewMacro_H
#define INCLUDED_SwgCuiNewMacro_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

//----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class UIButton;
class UIImage;
class UIPage;
class UIText;
class UIVolumePage;
class UIWidget;

//----------------------------------------------------------------------

class SwgCuiNewMacro : public CuiMediator, public UIEventCallback
{
public:
	explicit                      SwgCuiNewMacro    (UIPage & page);

	void                          performActivate   ();
	void                          performDeactivate ();
	static SwgCuiNewMacro *       createInto        (UIPage & parent);
	void                          setParams         (const Unicode::String& params);
	void                          OnButtonPressed   (UIWidget * context);

private:
	                             ~SwgCuiNewMacro ();
	                              SwgCuiNewMacro (const SwgCuiNewMacro & rhs);
	SwgCuiNewMacro &              operator= (const SwgCuiNewMacro & rhs);
	MessageDispatch::Callback *   m_callback;

	UIButton*                     m_okButton;
	UIButton*                     m_cancelButton;
	UIText*                       m_nameText;
	UIText*                       m_textText;
	UIVolumePage *                m_iconVolume;
	UIImage *                     m_sampleIcon;

	std::string                   m_macroCommandName;
};

//======================================================================

#endif

