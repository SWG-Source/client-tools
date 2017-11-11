// ======================================================================
//
// SwgCuiResourceSplitter.cpp
// Copyright Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiResourceSplitter.h"

#include "UIButton.h"
#include "UIMessage.h"
#include "UISliderbar.h"
#include "UIText.h"
#include "UITextbox.h"
#include "clientGame/ClientCommandQueue.h"
#include "clientGame/FactoryObject.h"
#include "clientGame/ResourceContainerObject.h"
#include "clientGame/ResourceTypeManager.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedFoundation/Crc.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/NetworkIdManager.h"

// ======================================================================
//
// SwgCuiResourceSplitter
//
// ======================================================================

//-----------------------------------------------------------------

SwgCuiResourceSplitter::SwgCuiResourceSplitter(UIPage &page)
: CuiMediator                  ("SwgCuiResourceSplitter", page)
, UIEventCallback              ()
, m_callBack                   (new MessageDispatch::Callback)
, m_splitAmountTextBox         (NULL)
, m_splitMaxText               (NULL)
, m_splitAmountSlider          (NULL)
, m_okButton                   (NULL)
, m_cancelButton               (NULL)
, m_inventoryNetworkId         ()
, m_containerNetworkId         ()
, m_viewer                     (NULL)
, m_initialize                 (true),
m_textName                     (0)
{
	setState (MS_closeable);
	setState (MS_closeDeactivates);

	m_callBack->connect (*this, &SwgCuiResourceSplitter::onSplitContainer, static_cast<CuiInventoryManager::Messages::SplitContainer *> (0));
	getCodeDataObject (TUITextbox,   m_splitAmountTextBox, "textboxAmount");
	getCodeDataObject (TUIText,      m_splitMaxText,       "max");
	getCodeDataObject (TUISliderbar, m_splitAmountSlider,  "slider");
	getCodeDataObject (TUIButton,    m_okButton,           "buttonOk");
	getCodeDataObject (TUIButton,    m_cancelButton,       "buttonCancel");
	getCodeDataObject (TUIText,      m_textName,           "textName");

	m_textName->SetPreLocalized (true);

	registerMediatorObject (*m_cancelButton,       true);
	registerMediatorObject (*m_okButton,           true);
	registerMediatorObject (*m_splitAmountSlider,  true);
	registerMediatorObject (*m_splitMaxText,       true);
	registerMediatorObject (*m_splitAmountTextBox, true);


	{
		UIWidget * widget = 0;
		getCodeDataObject (TUIWidget, widget, "viewer", true);
		if (widget)
		{
			m_viewer = NON_NULL (dynamic_cast<CuiWidget3dObjectListViewer *>(widget)); 

			if (m_viewer)
			{
				m_viewer->setCameraLodBias (2.0f);
				m_viewer->SetLocalTooltip    (CuiStringIds::tooltip_viewer_3d_controls.localize ());
				m_viewer->setAutoZoomOutOnly       (false);
				m_viewer->setCameraZoomInWhileTurn (false);
				m_viewer->setAlterObjects          (false);
				m_viewer->setCameraLookAtCenter    (true);
				m_viewer->setDragYawOk             (true);
				m_viewer->setPaused                (false);
				m_viewer->SetDragable              (false);	
				m_viewer->SetContextCapable        (true, false);
				m_viewer->setRotateSpeed           (1.0f);
				m_viewer->setCameraForceTarget     (false);
				m_viewer->setCameraTransformToObj  (true);
				m_viewer->setCameraLodBias         (3.0f);
				m_viewer->setCameraLodBiasOverride (true);
			}
		}
	}
}

//-----------------------------------------------------------------

SwgCuiResourceSplitter::~SwgCuiResourceSplitter()
{
	delete m_callBack;
	m_callBack = NULL;

	m_splitAmountTextBox = NULL;
	m_splitMaxText = NULL;
	m_splitAmountSlider = NULL;
	m_okButton = NULL;
	m_cancelButton = NULL;
	m_viewer = NULL;
}

//-----------------------------------------------------------------

void SwgCuiResourceSplitter::performActivate()
{
	if (m_initialize)
	{
		initialize();
	}

	setIsUpdating(true);
}

//-----------------------------------------------------------------

void SwgCuiResourceSplitter::performDeactivate()
{
	setIsUpdating(false);
}

//-----------------------------------------------------------------

void SwgCuiResourceSplitter::OnButtonPressed(UIWidget *context)
{
	if (context == m_okButton)
	{
		requestSplit();
		deactivate();
	}
	else if (context == m_cancelButton)
	{
		deactivate();
	}
}

//-----------------------------------------------------------------

void SwgCuiResourceSplitter::OnSliderbarChanged (UIWidget *context)
{
	if (context == m_splitAmountSlider)
	{
		// Set the text box to match the slider value

		char text[256];
		snprintf(text, sizeof(text), "%d", static_cast<int>(m_splitAmountSlider->GetValue()));
		m_splitAmountTextBox->SetLocalText(Unicode::narrowToWide(text));
	}
}

//-----------------------------------------------------------------

void SwgCuiResourceSplitter::onSplitContainer(CuiInventoryManager::Messages::SplitContainer::Payload const &payload)
{
	m_inventoryNetworkId = payload.first;
	m_containerNetworkId = payload.second;

	Object * const containerObject = NetworkIdManager::getObjectById(m_containerNetworkId);
	ResourceContainerObject * const resourceContainerObject = dynamic_cast<ResourceContainerObject *>(containerObject);
	FactoryObject * const factoryObject = dynamic_cast<FactoryObject *>(containerObject);

	if (resourceContainerObject == NULL && factoryObject == NULL)
	{
		deactivate();
		return;
	}

	// Set the slider min/max

	int containerQuantity;
	if (resourceContainerObject != NULL)
		containerQuantity = resourceContainerObject->getQuantity();
	else
		containerQuantity = factoryObject->getCount();

	if (containerQuantity <= 1)
	{
		deactivate();
		return;
	}

	m_splitAmountSlider->SetLowerLimit (1);
	m_splitAmountSlider->SetUpperLimit (containerQuantity - 1);
	m_splitAmountSlider->SetValue      (1, false);

	// If the resource container is lost, close this dialog

	setAssociatedObjectId             (m_containerNetworkId);

	m_viewer->setObject (containerObject);

	Unicode::String textName;
	if (resourceContainerObject != NULL)
		ResourceTypeManager::createTypeDisplayLabel (resourceContainerObject->getResourceType (), textName);
	else
	{
		const ClientObject * clientObj = factoryObject->getContainedObject ();
		if (clientObj)
			textName = clientObj->getLocalizedName();
		else
			textName = factoryObject->getLocalizedName ();
	}

	m_textName->SetLocalText (textName);

	if (isActive())
	{
		initialize();
	}
	else
	{
		m_initialize = true;
	}
}

//-----------------------------------------------------------------

void SwgCuiResourceSplitter::initialize()
{
	m_splitAmountTextBox->SetLocalText(Unicode::narrowToWide("1"));
	//pre-select text and push carat to the line end (so any typing overwrites the 1)
	m_splitAmountTextBox->SelectAll();
	m_splitAmountTextBox->MoveCaratToEndOfLine();


	// Set the max quantity
	Object * const containerObject = NetworkIdManager::getObjectById(m_containerNetworkId);
	ResourceContainerObject * const resourceContainerObject = dynamic_cast<ResourceContainerObject *>(containerObject);
	FactoryObject * const factoryObject = dynamic_cast<FactoryObject *>(containerObject);

	if (resourceContainerObject != NULL || factoryObject != NULL)
	{
		char text[256];
		if (resourceContainerObject != NULL)
			snprintf(text, sizeof(text), "%d", resourceContainerObject->getQuantity() - 1);
		else
			snprintf(text, sizeof(text), "%d", factoryObject->getCount() - 1);
		m_splitMaxText->SetLocalText(Unicode::narrowToWide(text));
	}
}

//-----------------------------------------------------------------

bool SwgCuiResourceSplitter::OnMessage(UIWidget *context, const UIMessage &message)
{
	bool result = true;

	switch (message.Type)
	{
		case UIMessage::KeyDown:
			{
				if (message.Keystroke == UIMessage::Enter)
				{
					if (context == m_splitAmountTextBox)
					{
						requestSplit();
						deactivate();

						result = false;
					}
				}
			}
			break;
	}

	return result;
}

//-----------------------------------------------------------------

void SwgCuiResourceSplitter::requestSplit()
{
	int const splitAmount = clamp(0, Unicode::toInt(m_splitAmountTextBox->GetLocalText()), static_cast<int>(m_splitAmountSlider->GetUpperLimit()));

	if (splitAmount >= 1)
	{
		static uint32 const resourceContainerHash = Crc::normalizeAndCalculate("resourceContainerSplit");
		static uint32 const factoryObjectHash = Crc::normalizeAndCalculate("factoryCrateSplit");
		
		// @todo: do we need to dectect if we're splitting a container in the world?
		char text[1024];
		_snprintf(text, sizeof(text), "%d %s -1 0 0 0", splitAmount, m_inventoryNetworkId.getValueString().c_str());
		
		uint32 hash;
		ResourceContainerObject * const resourceContainerObject = dynamic_cast<ResourceContainerObject *>(NetworkIdManager::getObjectById(m_containerNetworkId));
		if (resourceContainerObject != NULL)
			hash = resourceContainerHash;
		else
			hash = factoryObjectHash;

		ClientCommandQueue::enqueueCommand(hash, m_containerNetworkId, Unicode::narrowToWide(text));
	}
}

// ======================================================================
