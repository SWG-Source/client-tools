//======================================================================
//
// SwgCuiResourceExtraction_Hopper.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction_Hopper.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction_Hopper_TableModel.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction_Quantity.h"

#include "sharedMessageDispatch/Transceiver.h"

#include "clientGame/ClientInstallationSynchronizedUi.h"
#include "clientGame/InstallationObject.h"
#include "clientGame/ResourceIconManager.h"
#include "clientUserInterface/CuiIconManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiResourceManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "clientUserInterface/CuiStringIdsResource.h"

#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UITable.h"
#include "UITableModelDefault.h"
#include "UIText.h"
#include "UIUtils.h"
#include "UIVolumePage.h"

#include "UnicodeUtils.h"

#include <algorithm>
#include <cstdio>
#include <list>

//======================================================================

namespace SwgCuiResourceExtractionHopperNamespace
{
	const int maxRetrievable = 100000;

	const char * const nameText        = "InnerPage.NameText";
	const char * const efficiencyText  = "InnerPage.ResourceBar.EfficiencyText";
	const char * const barHolster      = "InnerPage.ResourceBar.EfficiencyHolster";
	const char * const efficiencyBar   = "InnerPage.ResourceBar.EfficiencyHolster.Bar";

	const float maxRange = 32.0f;

	void onMessageBoxDumpAllCancel (const CuiMessageBox & box)
	{
		if (box.completedAffirmative ())
		{
			CuiResourceManager::dumpAll();
		}
	}
}

using namespace SwgCuiResourceExtractionHopperNamespace;

//======================================================================

SwgCuiResourceExtraction_Hopper::SwgCuiResourceExtraction_Hopper (UIPage & page) :
CuiMediator        ("SwgCuiResourceExtraction_Hopper", page),
UIEventCallback    (),
m_buttonOk         (0),
m_buttonDiscard    (0),
m_buttonRetrieve   (0),
m_buttonDumpAll    (0),
m_textDisplay      (0),
m_hopperTable      (0),
m_mediatorQuantity (0),
m_sampleResource   (0),
m_hopperContents   (new HopperContentsVector),
m_resourceTableModel(0),
m_resourceIcons     (),
m_resources         (),
m_resourceHoppers   (),
m_callback      (new MessageDispatch::Callback)
{
	getCodeDataObject (TUIButton,     m_buttonOk,          "buttonOk");
	getCodeDataObject (TUIButton,     m_buttonDiscard,     "buttonDump");
	getCodeDataObject (TUIButton,     m_buttonRetrieve,    "buttonRetrieve");
	getCodeDataObject (TUIText,       m_textDisplay,       "textDisplay");
	getCodeDataObject (TUITable,      m_hopperTable,       "hopperTable");
	getCodeDataObject (TUIButton,     m_buttonDumpAll,     "buttonDumpAll");

	m_resourceTableModel = new TableModel(*this);
	m_resourceTableModel->Attach (0);
	m_resourceTableModel->SetName("HopperTableModel");
	getPage().AddChild(m_resourceTableModel);
	m_hopperTable->SetTableModel (m_resourceTableModel);
	m_resourceTableModel->updateTableColumnSizes (*m_hopperTable);
	m_hopperTable->SetCellHeight(48);

	m_resourceTableModel->fireDataChanged();
	m_resourceTableModel->fireColumnsChanged();

	IGNORE_RETURN(setState(MS_closeable));
	IGNORE_RETURN(setState(MS_closeDeactivates));
}

//----------------------------------------------------------------------

SwgCuiResourceExtraction_Hopper::~SwgCuiResourceExtraction_Hopper ()
{
	delete m_callback;
	m_callback = 0;

	delete m_hopperContents;
	m_hopperContents = 0;

	m_resourceTableModel->Detach (0);
	m_resourceTableModel = 0;

	m_buttonOk          = 0;
	m_buttonDiscard     = 0;
	m_buttonDumpAll     = 0;
	m_buttonRetrieve    = 0;
	m_textDisplay       = 0;
	m_hopperTable       = 0;
	m_sampleResource    = 0;
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_Hopper::performActivate ()
{
	update ();

	CuiResourceManager::requestResourceData ();

	IGNORE_RETURN(CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_ResourceExtraction_Quantity));

	m_hopperTable->AddCallback    (this);

	getPage ().SetFocus ();

	m_buttonOk->AddCallback       (this);
	m_buttonDiscard->AddCallback  (this);
	m_buttonRetrieve->AddCallback (this);
	m_buttonDumpAll->AddCallback  (this);
	m_resourceTableModel->fireDataChanged();
	m_resourceTableModel->fireColumnsChanged();

	InstallationObject * const harvester = CuiResourceManager::getHarvesterInstallation ();
	if(!harvester)
	{
		DEBUG_WARNING(true, ("No harvester"));
		return;
	}

	setAssociatedObjectId(harvester->getNetworkId());
	setMaxRangeFromObject(maxRange);

	m_callback->connect (*this, &SwgCuiResourceExtraction_Hopper::onEmptyCompleted, static_cast<CuiResourceManager::Messages::EmptyCompleted *> (0));
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_Hopper::performDeactivate ()
{
	m_callback->disconnect (*this, &SwgCuiResourceExtraction_Hopper::onEmptyCompleted, static_cast<CuiResourceManager::Messages::EmptyCompleted *> (0));

	m_hopperTable->RemoveCallback    (this);
	m_buttonOk->RemoveCallback       (this);
	m_buttonDiscard->RemoveCallback  (this);
	m_buttonRetrieve->RemoveCallback (this);
	m_buttonDumpAll->RemoveCallback  (this);

	IGNORE_RETURN(CuiMediatorFactory::deactivateInWorkspace (CuiMediatorTypes::WS_ResourceExtraction_Quantity));

	clearResources ();

	setIsUpdating(false);
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_Hopper::onEmptyCompleted (const bool & )
{
	update ();
	CuiResourceManager::requestResourceData ();
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_Hopper::OnButtonPressed    (UIWidget *context)
{
	if (context == m_buttonOk)
	{
		deactivate ();
	}
	else if (context == m_buttonDiscard)
	{
		const int index = m_hopperTable->GetLastSelectedRow ();
		if (index >= 0 && index < static_cast<int>(m_hopperContents->size ()))
		{
			const HopperContentElement & elem = (*m_hopperContents) [static_cast<unsigned int>(index)];
			m_mediatorQuantity = safe_cast<SwgCuiResourceExtraction_Quantity*>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_ResourceExtraction_Quantity));
			m_mediatorQuantity->setInfo (elem.first, elem.second, true);
			m_resourceTableModel->fireDataChanged();
			m_resourceTableModel->fireColumnsChanged();
			m_hopperTable->RemoveRowSelection(index);
		}
		else
			CuiMessageBox::createInfoBox (CuiStringIdsResource::res_pleaseselectresource.localize());
	}

	else if (context == m_buttonRetrieve)
	{
		const int index = m_hopperTable->GetLastSelectedRow ();
		if (index >= 0 && index < static_cast<int>(m_hopperContents->size ()))
		{
			int retrievable = SwgCuiResourceExtractionHopperNamespace::maxRetrievable;
			const HopperContentElement & elem = (*m_hopperContents) [static_cast<unsigned int>(index)];
			if (elem.second < SwgCuiResourceExtractionHopperNamespace::maxRetrievable)
				retrievable = static_cast<int>(floor(elem.second));
			
			m_mediatorQuantity = safe_cast<SwgCuiResourceExtraction_Quantity*>(CuiMediatorFactory::activateInWorkspace (CuiMediatorTypes::WS_ResourceExtraction_Quantity));
			m_mediatorQuantity->setInfo (elem.first, static_cast<float>(retrievable), false);
			m_resourceTableModel->fireDataChanged();
			m_resourceTableModel->fireColumnsChanged();
			m_hopperTable->RemoveRowSelection(index);
		}
		else
			CuiMessageBox::createInfoBox (CuiStringIdsResource::res_pleaseselectresource.localize());
	}

	else if (context == m_buttonDumpAll)
	{
		CuiMessageBox::createYesNoBox (CuiStringIdsResource::res_confirm_dumpall_hopper.localize (), onMessageBoxDumpAllCancel);
	}
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_Hopper::updateHopperContentBarPage (const std::pair<NetworkId, uint16> & elem, float, int maxContents, const UIWidget * page) const
{
	if (!page)
		return;

	const CuiResourceManager::ResourceInfo * const info = CuiResourceManager::findResourceInfo (elem.first);
	
	UIText * const text = GET_UI_OBJ ((*page), UIText, nameText);

	if (text)
		text->SetText (info ? info->name : Unicode::String ());

	UIText * const effText = GET_UI_OBJ ((*page), UIText, efficiencyText);
	if (effText)
	{
		char buf [128];
		IGNORE_RETURN (_snprintf (buf, 128, "%d", elem.second));
		effText->SetText (Unicode::narrowToWide (buf));
	}
	
	UIWidget * const holster = GET_UI_OBJ ((*page), UIWidget, barHolster);
	const long totalLength = holster ? (holster->IsA (TUIWidget) ? safe_cast<const UIWidget *>(holster)->GetWidth () : 0) : 0;
	
	{
		UIWidget * const bar = GET_UI_OBJ ((*page), UIWidget, efficiencyBar);
		
		if (bar)
		{
			if (maxContents)
				bar->SetWidth (elem.second * totalLength / maxContents);
			else
				bar->SetWidth (0L);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_Hopper::update ()
{
	const CuiResourceManager::Synchronized * sync = CuiResourceManager::getSynchronized ();

	clearResources ();

	m_hopperContents->clear ();

	if (sync)
	{
		const ClientInstallationSynchronizedUi::HopperContentsVector & hopper = sync->getHopperContentsVector ();

		//-- take a snapshot of the hopper
		*m_hopperContents = hopper;

		for (ClientInstallationSynchronizedUi::HopperContentsVector::const_iterator it = hopper.begin (); it != hopper.end (); ++it)
		{
			const ClientInstallationSynchronizedUi::HopperContentElement & elem = *it;

			UIPage * child = 0;

			char buf [64];
			IGNORE_RETURN (_itoa (static_cast<int>(elem.first.getValue ()), buf, 10));

			//create an icon
			Object* obj = ResourceIconManager::getObjectForType(elem.first);
			CuiWidget3dObjectListViewer * viewer = 0;
			if(obj)
			{
				viewer = CuiIconManager::createObjectIcon(*obj, 0);
				//the UI owns this, so it needs to alter it
				viewer->setAlterObjects(true);
				viewer->SetDragable             (false);
				viewer->SetContextCapable       (false, false);
				viewer->setRotateSpeed          (0.0f);
				viewer->Attach (0);
				m_resourceIcons.push_back(viewer);
			}
			else
			{
				DEBUG_WARNING(true, ("Could not build an icon object for resource %d", elem.first.getValue()));
				m_resourceIcons.push_back(NULL);
			}
			m_resources.push_back(elem.first);

			updateHopperContentBarPage (elem, sync->getHopperContentsCur(), sync->getHopperContentsMax(), child);
		}

		const float hopperContentsCur = sync->getHopperContentsCur ();
		const int hopperContentsMax = sync->getHopperContentsMax ();

		char buf [128];
		IGNORE_RETURN (_snprintf (buf, 128, "%d/%d", static_cast<int>(floor(hopperContentsCur)), hopperContentsMax));
		m_textDisplay->SetText (Unicode::narrowToWide (buf));
	}

	m_resourceTableModel->fireColumnsChanged();
	m_resourceTableModel->fireDataChanged();
}

//----------------------------------------------------------------------

NetworkId SwgCuiResourceExtraction_Hopper::getResourceAtPosition (int index)
{
	if(index >= getNumResources())
	{
		DEBUG_WARNING(true, ("Asked for for resource index %s when max is %d", index, getNumResources()));
		return NetworkId::cms_invalid;
	}
	return m_resources[static_cast<unsigned int>(index)];
}

//----------------------------------------------------------------------

UIPage* SwgCuiResourceExtraction_Hopper::getResourceHopperAtPosition (int index)
{
	if(index >= getNumResources())
	{
		DEBUG_WARNING(true, ("Asked for for resource index %s when max is %d", index, getNumResources()));
		return 0;
	}
	return m_resourceHoppers[static_cast<unsigned int>(index)];
}

//----------------------------------------------------------------------

CuiWidget3dObjectListViewer * SwgCuiResourceExtraction_Hopper::getResourceIconAtPosition (int index)
{
	if(index >= getNumResources())
	{
		DEBUG_WARNING(true, ("Asked for for resource index %s when max is %d", index, getNumResources()));
		return 0;
	}
	return m_resourceIcons[static_cast<unsigned int>(index)];
}

//----------------------------------------------------------------------

int SwgCuiResourceExtraction_Hopper::getNumResources () const
{
	return static_cast<int>(m_resources.size());
}

//----------------------------------------------------------------------

float SwgCuiResourceExtraction_Hopper::getHopperSizeForResource  (const NetworkId& resource)
{
	for(HopperContentsVector::iterator i = m_hopperContents->begin(); i != m_hopperContents->end(); ++i)
	{
		if(i->first == resource)
			return i->second;
	}
	return 0;
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction_Hopper::clearResources ()
{
	m_resources.clear();
	//Nulls can theoretically exist in this vector, since we want to maintain vectors of the same size
	for(std::vector<CuiWidget3dObjectListViewer*>::iterator i = m_resourceIcons.begin (); i != m_resourceIcons.end (); ++i)
	{
		if(*i)
			(*i)->Detach(0);
	}
	m_resourceIcons.clear();
	m_resourceHoppers.clear();
}

//-----------------------------------------------------------------------

bool SwgCuiResourceExtraction_Hopper::OnMessage(UIWidget *context, const UIMessage & msg)
{
	if (msg.Type == UIMessage::LeftMouseDoubleClick)
	{
		if(context == m_hopperTable)
			m_buttonRetrieve->Press();
	}

	return true;
}

//----------------------------------------------------------------------
