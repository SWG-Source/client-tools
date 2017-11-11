//======================================================================
//
// SwgCuiLootBox.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiLootBox.h"


#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/NetworkIdManager.h"
#include "UIButton.h"
#include "UIComposite.h"
#include "UIText.h"

//----------------------------------------------------------------------

namespace SwgCuiLootBoxNamespace
{
	std::string cms_item("item");
}

using namespace SwgCuiLootBoxNamespace;

//----------------------------------------------------------------------

SwgCuiLootBox::SwgCuiLootBox(UIPage & page) :
CuiMediator                 ("SwgCuiLootBox", page)
 , UIEventCallback          ()
 , m_callback               (new MessageDispatch::Callback)
 , m_okButton               (NULL)
 , m_comp                   (NULL)
 , m_sampleItem             (NULL)
 , m_endSpacer              (NULL)
{
	getCodeDataObject(TUIButton,         m_okButton,          "buttonOk");
	getCodeDataObject(TUIComposite,      m_comp,              "comp");
	getCodeDataObject(TUIComposite,      m_sampleItem,        "sampleItem");
	getCodeDataObject(TUIText,           m_endSpacer,         "endspacer");

	registerMediatorObject (*m_okButton, true);

	m_sampleItem->Link();

	clearRewards();

	setState (MS_closeable);
	setState (MS_closeDeactivates);
}

//----------------------------------------------------------------------

SwgCuiLootBox::~SwgCuiLootBox ()
{
	delete m_callback;
	m_callback = 0;

	m_okButton   = NULL;
	m_comp       = NULL;
	m_sampleItem = NULL;
}

//----------------------------------------------------------------------

void SwgCuiLootBox::performActivate   ()
{
	CuiManager::requestPointer(true);
}

//----------------------------------------------------------------------

void SwgCuiLootBox::performDeactivate ()
{
	CuiManager::requestPointer(false);
}

//----------------------------------------------------------------------

void SwgCuiLootBox::OnButtonPressed (UIWidget * context)
{
	if(context == m_okButton)
	{
		closeThroughWorkspace();
	}
}

//----------------------------------------------------------------------

/**
 * Delete all existing rewards, clear for new use.
 */
void SwgCuiLootBox::clearRewards()
{
	//all task pages are named "task" so find and delete them
	UIBaseObject * taskPage = m_comp->GetChild(cms_item.c_str());
	while(taskPage)
	{
		m_comp->RemoveChild(taskPage);
		taskPage = m_comp->GetChild(cms_item.c_str());
	}
}

//----------------------------------------------------------------------

void SwgCuiLootBox::addReward(NetworkId const & objectNid)
{
	//dupe the sample and grab the necessary pieces
	UIComposite * const dupePage = safe_cast<UIComposite *>(m_sampleItem->DuplicateObject());
	NOT_NULL (dupePage);
	dupePage->SetVisible(true);
	UIText * const rewardName = dynamic_cast<UIText *>(dupePage->GetChild("rewardName"));
	NOT_NULL(rewardName);
	rewardName->Clear();
	CuiWidget3dObjectListViewer * const rewardItemViewer = dynamic_cast<CuiWidget3dObjectListViewer *>(dupePage->GetChild("icon.v"));
	NOT_NULL(rewardItemViewer);
	rewardItemViewer->setRotateSpeed(1.0f);

	rewardItemViewer->setObject(NULL);
	Object * const theObject = NetworkIdManager::getObjectById(objectNid);
	if(theObject)
	{
		rewardItemViewer->setObject(theObject);
		rewardItemViewer->recomputeZoom();
		rewardItemViewer->setViewDirty(true);
		ClientObject const * const co = theObject->asClientObject();
		if(co)
		{
			rewardName->SetLocalText(co->getLocalizedName());
		}
	}

	dupePage->SetName(cms_item);
	IGNORE_RETURN(m_comp->InsertChildBefore(dupePage, m_endSpacer));
	dupePage->Link();

}
//----------------------------------------------------------------------
