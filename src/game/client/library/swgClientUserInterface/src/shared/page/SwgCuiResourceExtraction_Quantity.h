//======================================================================
//
// SwgCuiResourceExtraction_Quantity.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiResourceExtraction_Quantity_H
#define INCLUDED_SwgCuiResourceExtraction_Quantity_H

#include "UINotification.h"
#include "sharedMessageDispatch/Receiver.h"
#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

class UISliderbar;
class UIButton;
class UITextbox;
class UIText;

namespace MessageDispatch
{
	class Callback;
}

//======================================================================

class SwgCuiResourceExtraction_Quantity :
public CuiMediator,
public UIEventCallback,
public UINotification
{
public:

	explicit                SwgCuiResourceExtraction_Quantity (UIPage & page);

	void                    OnSliderbarChanged (UIWidget *context);
	void                    OnButtonPressed    (UIWidget *context);
	void                    OnTextboxChanged   (UIWidget *context);

	void                    setInfo (const NetworkId & resourceId, float amount, bool discard);

	void                    Notify( UINotificationServer *notifyingObject, UIBaseObject *contextObject, Code notificationCode );

	void                    onEmptyCompleted (const bool & b);

protected:
	void                    performActivate ();
	void                    performDeactivate ();

private:
	~SwgCuiResourceExtraction_Quantity ();
	SwgCuiResourceExtraction_Quantity ();
	SwgCuiResourceExtraction_Quantity (const SwgCuiResourceExtraction_Quantity & rhs);
	SwgCuiResourceExtraction_Quantity & operator= (const SwgCuiResourceExtraction_Quantity & rhs);


	NetworkId               m_resourceId;

	UIButton *              m_buttonOk;
	UIButton *              m_buttonCancel;
	UITextbox *             m_textbox;
	UISliderbar *           m_slider;
	UIText *                m_textName;

	UIText *                m_textRetrieval;
	UIText *                m_textDiscard;

	bool                    m_ignoreTextboxChange;

	bool                    m_discard;

	bool                    m_transition;
	float                   m_amount;

	MessageDispatch::Callback * m_callback;
};

#endif

//======================================================================
