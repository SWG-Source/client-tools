//======================================================================
//
// SwgCuiResourceExtraction_Quantity.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction_Quantity.h"

#include "UIButton.h"
#include "UIClock.h"
#include "UIData.h"
#include "UISliderbar.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UIUtils.h"
#include "UIPage.h"

#include "sharedMessageDispatch/Transceiver.h"

#include "clientGame/InstallationObject.h"
#include "clientUserInterface/CuiResourceManager.h"

//======================================================================

namespace SwgCuiResourceExtraction_QuantityNamespace
{
	const float maxRange = 32.0f;
}

using namespace SwgCuiResourceExtraction_QuantityNamespace;

//======================================================================

SwgCuiResourceExtraction_Quantity::SwgCuiResourceExtraction_Quantity (UIPage & page) :
CuiMediator     ("SwgCuiResourceExtraction_Quantity", page),
UIEventCallback (),
UINotification  (),
m_resourceId    (),
m_buttonOk      (0),
m_buttonCancel  (0),
m_textbox       (0),
m_slider        (0),
m_textName      (0),
m_textRetrieval (0),
m_textDiscard   (0),
m_ignoreTextboxChange (false),
m_discard       (false),
m_transition    (false),
m_amount        (0.0f),
m_callback      (new MessageDispatch::Callback)
{
	getCodeDataObject (TUIButton,     m_buttonOk,          "buttonOk");
	getCodeDataObject (TUIButton,     m_buttonCancel,      "buttonCancel");
	getCodeDataObject (TUISliderbar,  m_slider,            "slider");
	getCodeDataObject (TUITextbox,    m_textbox,           "box.textBox");
	getCodeDataObject (TUIText,       m_textName,          "textResourceName");

	getCodeDataObject (TUIText,       m_textRetrieval,     "textRetrieval");
	getCodeDataObject (TUIText,       m_textDiscard,       "textDiscard");
}

//----------------------------------------------------------------------

SwgCuiResourceExtraction_Quantity::~SwgCuiResourceExtraction_Quantity ()
{
	delete m_callback;
	m_callback = 0;

	m_buttonOk      = 0;
	m_buttonCancel  = 0;
	m_textbox       = 0;
	m_slider        = 0;
	m_textName      = 0;
}

//----------------------------------------------------------------------

void            SwgCuiResourceExtraction_Quantity::performActivate ()
{
	m_transition = false;

	m_buttonOk->AddCallback     (this);
	m_buttonCancel->AddCallback (this);
	m_textbox->AddCallback      (this);
	m_slider->AddCallback       (this);

	InstallationObject * const harvester = CuiResourceManager::getHarvesterInstallation ();
	if(!harvester)
	{
		DEBUG_WARNING(true, ("No harvester"));
		return;
	}

	setAssociatedObjectId(harvester->getNetworkId());
	setMaxRangeFromObject(maxRange);

	m_callback->connect (*this, &SwgCuiResourceExtraction_Quantity::onEmptyCompleted, static_cast<CuiResourceManager::Messages::EmptyCompleted *> (0));
}

//----------------------------------------------------------------------

void            SwgCuiResourceExtraction_Quantity::performDeactivate ()
{
	m_callback->disconnect (*this, &SwgCuiResourceExtraction_Quantity::onEmptyCompleted, static_cast<CuiResourceManager::Messages::EmptyCompleted *> (0));

	m_buttonOk->RemoveCallback     (this);
	m_buttonCancel->RemoveCallback (this);
	m_textbox->RemoveCallback      (this);
	m_slider->RemoveCallback       (this);

	setIsUpdating(false);
}

//----------------------------------------------------------------------

void  SwgCuiResourceExtraction_Quantity::OnSliderbarChanged (UIWidget * context)
{
	if(context == m_slider)
	{
		const long value = m_slider->GetValue ();
		UIString str;
		IGNORE_RETURN(UIUtils::FormatLong (str, value));
		m_ignoreTextboxChange = true;
		m_textbox->SetText (str);
		m_ignoreTextboxChange = false;
	}
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_Quantity::OnTextboxChanged   (UIWidget * context)
{
	if(context == m_textbox)
	{		
		if (m_ignoreTextboxChange)
			return;
		const long value = m_textbox->GetNumericIntegerValue ();
		m_slider->SetValue (value, false);
	}
}

//----------------------------------------------------------------------

void  SwgCuiResourceExtraction_Quantity::OnButtonPressed    (UIWidget *context)
{
	if (context == m_buttonOk)
	{
		CuiResourceManager::emptyHopper (m_resourceId, m_textbox->GetNumericIntegerValue (), m_discard, false);
	}
	else if (context == m_buttonCancel)
	{
		deactivate ();
	}
}

//----------------------------------------------------------------------

void  SwgCuiResourceExtraction_Quantity::setInfo (const NetworkId & resourceId, float amount, bool discard)
{
	m_resourceId  = resourceId;
	m_discard     = discard;
	m_amount      = amount;

	m_slider->SetUpperLimit (static_cast<int>(floor(amount)));
	m_slider->SetValue      (static_cast<int>(floor(amount)), false);
	OnSliderbarChanged      (m_slider);

	m_textDiscard->SetVisible   (discard);
	m_textRetrieval->SetVisible (!discard);

	m_textName->SetPreLocalized (true);

	const CuiResourceManager::ResourceInfo * const info = CuiResourceManager::findResourceInfo (resourceId);
	if (info)
	{
		m_textName->SetLocalText (info->name);
	}
	else
	{
		m_textName->SetLocalText (Unicode::String ());
	}

	//update the slider bar (and from that, the textbox)
	OnSliderbarChanged(m_slider);
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_Quantity::onEmptyCompleted (const bool & )
{
	m_transition = true;
	UIClock::gUIClock ().ListenPerFrame (this);
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_Quantity::Notify( UINotificationServer *, UIBaseObject *, Code  )
{
	if (m_transition)
	{
		UIClock::gUIClock ().StopListening (this);
		m_transition = false;
		deactivate ();
	}
}

//======================================================================
