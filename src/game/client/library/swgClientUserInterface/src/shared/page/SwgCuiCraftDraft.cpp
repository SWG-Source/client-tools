//======================================================================
//
// SwgCuiCraftDraft.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiCraftDraft.h"

//======================================================================

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIDataSourceContainer.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UITabbedPane.h"
#include "UITreeView.h"
#include "UITreeView_DataNode.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientObject.h"
#include "clientGame/DraftSchematicInfo.h"
#include "clientGame/DraftSchematicManager.h"
#include "clientUserInterface/CuiCraftManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiStringIdsCraft.h"
#include "clientUserInterface/CuiStringTables.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueDraftSchematicsData.h"
#include "sharedNetworkMessages/MessageQueueDraftSlots.h"
#include "sharedObject/ObjectTemplateList.h"
#include "swgClientUserInterface/SwgCuiInventoryInfo.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"

#include <map>
#include <vector>

//======================================================================

namespace
{
	namespace Properties
	{
		const UILowerString Category  = UILowerString ("Category");
		const UILowerString GameType  = UILowerString ("GameType");
		const UILowerString Index     = UILowerString ("Index");
		const UILowerString CrcServer = UILowerString ("server_crc");
		const UILowerString CrcShared = UILowerString ("shared_crc");
	}
}

//----------------------------------------------------------------------

SwgCuiCraftDraft::InternalSchematicData::InternalSchematicData (const std::pair<uint32, uint32> & _crc, int _category, int _index) :
crc         (_crc),
displayName (),
gameType    (0),
category    (_category),
index       (_index)
{
	const DraftSchematicInfo * const info = DraftSchematicManager::findDraftSchematic (crc);

	if (!info)
		WARNING (true, ("SwgCuiCraftDraft received draft that player doesn't have [%lu]", crc.first));
	else
	{
		info->requestDataIfNeeded ();
		displayName              = info->getLocalizedName ();
		const ClientObject * const obj = info->getClientObject ();
		if (!obj)
			WARNING (true, ("Null obj for SwgCuiCraftDraft draft [%lu]", crc.first));
		else
			gameType = obj->getGameObjectType ();
	}
}

//----------------------------------------------------------------------

SwgCuiCraftDraft::InternalCategoryData::InternalCategoryData (int _category) :
displayName (),
category    (_category)
{
	static Crafting::StringVector sv;
	sv.clear ();
	Crafting::getCraftingTypeNames (category, sv);

	std::string categoryName;
	
	static StringId sid (CuiStringTables::craft_types, std::string ());
	if (sv.empty ())
		sid.setText ("unknown");
	else
		sid.setText (sv.back ());

	displayName = sid.localize ();
}

//----------------------------------------------------------------------

SwgCuiCraftDraft::InternalGameTypeData::InternalGameTypeData (int _gameType) :
displayName   (),
gameType      (_gameType)
{
	displayName = GameObjectTypes::getLocalizedName (gameType);
}

//----------------------------------------------------------------------

SwgCuiCraftDraft::SwgCuiCraftDraft (UIPage & page) :
CuiMediator                 ("SwgCuiCraftDraft", page),
UIEventCallback             (),
m_buttonBack                (0),
m_buttonNext                (0),
m_callback                  (new MessageDispatch::Callback),
m_schematics                (new SchematicCategoryMap),
m_tabs                      (0),
m_info                      (0),
m_selectedCategory          (0),
m_treeView                  (0),
m_lastCategorySelected      (0),
m_checkShowAll              (0),
m_lastSchematicSelectedCrcs (new CategoryCrcMap)
{
	setState    (MS_closeable);

	getCodeDataObject (TUIButton,     m_buttonBack,     "buttonBack");
	getCodeDataObject (TUIButton,     m_buttonNext,     "buttonNext");
	getCodeDataObject (TUITabbedPane, m_tabs,           "tabs");

	getCodeDataObject (TUITreeView,   m_treeView,       "treeView");
	getCodeDataObject (TUICheckbox,   m_checkShowAll,   "checkShowAll");
	
	{
		UIPage * infoPage = 0;
		getCodeDataObject (TUIPage, infoPage, "info");
		m_info = new SwgCuiInventoryInfo (*infoPage, true);
		m_info->fetch ();			
	}

	registerMediatorObject (*m_buttonBack,    true);
	registerMediatorObject (*m_buttonNext,    true);
	registerMediatorObject (*m_tabs,          true);
	registerMediatorObject (*m_treeView,      true);
	registerMediatorObject (*m_checkShowAll,  true);
}

//----------------------------------------------------------------------

SwgCuiCraftDraft::~SwgCuiCraftDraft ()
{
	m_checkShowAll  = 0;
	m_buttonBack    = 0;
	m_buttonNext    = 0;
	m_tabs          = 0;
	m_treeView      = 0;

	m_info->release ();
	m_info = 0;

	delete m_callback;
	m_callback = 0;

	delete m_schematics;
	m_schematics = 0;

	delete m_lastSchematicSelectedCrcs;
	m_lastSchematicSelectedCrcs = 0;
}

//----------------------------------------------------------------------

void SwgCuiCraftDraft::performActivate   ()
{	
	if (!CuiCraftManager::isCrafting ())
	{
		closeNextFrame ();
		return;
	}

	CuiManager::requestPointer (true);

	m_info->activate ();

	reset ();

	m_callback->connect (*this, &SwgCuiCraftDraft::onDraftSlotsReceived,         static_cast<CuiCraftManager::Messages::DraftSlotsReceived *>     (0));
	m_callback->connect (*this, &SwgCuiCraftDraft::onDraftSchematicsReceived,    static_cast<CuiCraftManager::Messages::DraftSchematicsReceived *>     (0));
	m_callback->connect (*this, &SwgCuiCraftDraft::onSessionEnded,               static_cast<CuiCraftManager::Messages::SessionEnded *>     (0));
}

//----------------------------------------------------------------------

void SwgCuiCraftDraft::performDeactivate ()
{
	CuiManager::requestPointer (false);

	m_info->deactivate ();

	m_callback->disconnect (*this, &SwgCuiCraftDraft::onDraftSlotsReceived,         static_cast<CuiCraftManager::Messages::DraftSlotsReceived *>     (0));
	m_callback->disconnect (*this, &SwgCuiCraftDraft::onDraftSchematicsReceived,    static_cast<CuiCraftManager::Messages::DraftSchematicsReceived *>     (0));
	m_callback->disconnect (*this, &SwgCuiCraftDraft::onSessionEnded,               static_cast<CuiCraftManager::Messages::SessionEnded *>     (0));

}

//----------------------------------------------------------------------

bool SwgCuiCraftDraft::close ()
{
	CuiCraftManager::stopCrafting (true);
	deactivate ();
	
	return false;
}

//----------------------------------------------------------------------

void SwgCuiCraftDraft::OnButtonPressed( UIWidget *context )
{
	if (context == m_buttonBack)
	{
		close ();
	}
	else if (context == m_buttonNext)
	{
		ok (false);
	}
}

//----------------------------------------------------------------------

bool SwgCuiCraftDraft::OnMessage( UIWidget * context, const UIMessage & msg)
{
	if (context == m_treeView)
	{
		if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			UITreeView::DataNode * node = 0;
			long row = 0;
			if (m_treeView->GetRowFromPoint (msg.MouseCoords, row, &node) && node)
			{
				if (node->selectable)
				{
					ok (true);
					return false;
				}
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiCraftDraft::OnGenericSelectionChanged ( UIWidget *context )
{
	if (context == m_treeView)
	{
		updateInfo ();
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftDraft::OnTabbedPaneChanged (UIWidget * context)
{
	if (context == m_tabs)
	{
		resetCategory ();
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftDraft::onDraftSlotsReceived (const CuiCraftManager::SlotVector &)
{
	deactivate ();
	CuiMediatorFactory::activate (CuiMediatorTypes::WS_CraftAssembly);
}

//----------------------------------------------------------------------

void SwgCuiCraftDraft::onDraftSchematicsReceived (const CuiCraftManager::SchematicVector &)
{
	reset ();
}

//----------------------------------------------------------------------

void SwgCuiCraftDraft::reset ()
{
	m_schematics->clear ();

	const CuiCraftManager::SchematicVector & sv = CuiCraftManager::getSchematics ();
	
	{
		int index = 0;
		for (CuiCraftManager::SchematicVector::const_iterator it = sv.begin (); it != sv.end (); ++it, ++index)
		{
			const CuiCraftManager::SchematicData & sd = *it;
			
			const InternalSchematicData internalSchematicData (std::make_pair(sd.serverCrc, sd.sharedCrc), sd.category, index);
			const InternalCategoryData  internalCategoryData  (internalSchematicData.category);
			const InternalGameTypeData  internalGameTypeData  (internalSchematicData.gameType);

			SchematicDataMap & sdm    = (*m_schematics) [internalCategoryData];
			SchematicDataVector & sdv = sdm [internalGameTypeData];
			sdv.push_back (internalSchematicData);
		}
	}
	
	int selectedIndex = 0;
	{
		UIDataSource * const ds = NON_NULL (m_tabs->GetDataSource ());
		ds->Attach (0);
		m_tabs->SetDataSource (0);
		ds->Clear ();
		
		bool selectedGeneric = false;

		int index = 0;
		for (SchematicCategoryMap::const_iterator it = m_schematics->begin (); it != m_schematics->end (); ++it, ++index)
		{
			const InternalCategoryData & categoryData = (*it).first;
			
			if (selectedGeneric || m_lastCategorySelected == categoryData.category)
			{
				selectedIndex = index;
				selectedGeneric = false;
			}
			else if (selectedIndex == index && categoryData.category == Crafting::CT_genericItem)
				selectedGeneric = true;

			UIData * const data = new UIData;
			data->SetName(FormattedString<256>().sprintf("%d", index));
			data->SetProperty        (UITabbedPane::DataProperties::Text, categoryData.displayName);
			data->SetProperty        (UITabbedPane::DataProperties::DATA_NAME, categoryData.displayName);
			data->SetPropertyInteger (Properties::Category, categoryData.category);
			ds->AddChild (data);
		}

		m_tabs->SetDataSource (ds);
		ds->Detach (0);
	}
	
	m_tabs->SetActiveTab (selectedIndex);
	OnTabbedPaneChanged (m_tabs);
}

//----------------------------------------------------------------------

void SwgCuiCraftDraft::resetCategory ()
{
	m_selectedCategory = 0;

	UIDataSourceContainer * const dsc_tree = NON_NULL (m_treeView->GetDataSourceContainer ());
	dsc_tree->Clear ();
	dsc_tree->Attach (0);
	m_treeView->SetDataSourceContainer (0);

	const int activeTab = m_tabs->GetActiveTab ();

	UIDataSourceContainer * dsc_toSelect = 0;

	if (activeTab >= 0)
	{
		const UIData * const tabData = m_tabs->GetTabData (activeTab);

		NOT_NULL (tabData);

		if (tabData->GetPropertyInteger (Properties::Category, m_selectedCategory))
		{
			if (m_selectedCategory)
				m_lastCategorySelected = m_selectedCategory;

			uint32 lastSchematicSelectedForCategory = 0;

			const CategoryCrcMap::const_iterator lastSchematicCrc = m_lastSchematicSelectedCrcs->find (m_selectedCategory);
			if (lastSchematicCrc != m_lastSchematicSelectedCrcs->end ())
				lastSchematicSelectedForCategory = (*lastSchematicCrc).second;

			const InternalCategoryData icd (m_selectedCategory);
			const SchematicCategoryMap::const_iterator it = m_schematics->find (icd);

			if (it != m_schematics->end ())
			{
				const SchematicDataMap & sdm = (*it).second;
				
				bool found = false;

				int index = 0;
				for (SchematicDataMap::const_iterator tit = sdm.begin (); tit != sdm.end (); ++tit, ++index)
				{
					const InternalGameTypeData & internalGameTypeData = (*tit).first;
					
					found = true;

					UIDataSourceContainer * const dsc_gameType = new UIDataSourceContainer;
					dsc_gameType->SetName(FormattedString<256>().sprintf("%d", index));
					dsc_gameType->SetProperty         (UITreeView::DataProperties::LocalText, internalGameTypeData.displayName);
					dsc_gameType->SetPropertyBoolean  (UITreeView::DataProperties::Selectable, false);

					const SchematicDataVector & sdv = (*tit).second;

					UIDataSourceContainer * const pop = populateGameTypeDraftData (sdv, *dsc_gameType, lastSchematicSelectedForCategory);
					if (pop && !dsc_toSelect)
						dsc_toSelect = pop;

					dsc_tree->AddChild (dsc_gameType);
				}
			}
		}
	}

	m_treeView->SetDataSourceContainer (dsc_tree);
	dsc_tree->Detach (0);

	m_treeView->SetAllRowsExpanded (true);

	if (dsc_toSelect)
	{
		int row = 0;
		if (m_treeView->FindDataNodeByDataSource (*dsc_toSelect, row))
			m_treeView->SelectRow (row);
	}
	else
		m_treeView->SelectRow (1);

	m_treeView->ScrollToRow (m_treeView->GetLastSelectedRow ());

	updateInfo ();
}

//----------------------------------------------------------------------

void SwgCuiCraftDraft::updateInfo ()
{
	const long row = m_treeView->GetLastSelectedRow ();
	const UIDataSourceContainer * const dsc = m_treeView->GetDataSourceContainerAtRow  (row);
	
	int index = -1;
	if (dsc && dsc->GetPropertyInteger (Properties::Index, index))
	{
		int32 serverCrc, sharedCrc;
		dsc->GetPropertyLong (Properties::CrcServer, serverCrc);
		dsc->GetPropertyLong (Properties::CrcShared, sharedCrc);
		const std::pair<uint32, uint32> & draftCrc = std::make_pair(serverCrc, sharedCrc);

		const DraftSchematicInfo * const info = DraftSchematicManager::findDraftSchematic (draftCrc);
		
		(*m_lastSchematicSelectedCrcs) [m_selectedCategory] = draftCrc.first;

		if (!info)
			WARNING (true, ("SwgCuiCraftDraft selected draft that player doesn't have [%lu]", draftCrc.first));
		else
		{
			ClientObject * const obj = info->getClientObject ();
			m_info->setInfoObject (obj, false);
		}
	}
	else
		m_info->setInfoObject (0, false);

}

//----------------------------------------------------------------------

void SwgCuiCraftDraft::ok (bool fromTree)
{
	const long row = m_treeView->GetLastSelectedRow ();
	const UIDataSourceContainer * const dsc = m_treeView->GetDataSourceContainerAtRow  (row);
	
	int index = -1;
	if (dsc && dsc->GetPropertyInteger (Properties::Index, index))
	{
		CuiCraftManager::requestDraftSlots (index);
	}
	else
	{
		if (!fromTree)
			CuiMessageBox::createInfoBox (CuiStringIdsCraft::err_no_draft_schematic_selected.localize ());
	}
}

//----------------------------------------------------------------------

void SwgCuiCraftDraft::onSessionEnded (const bool &)
{
	closeNextFrame ();
}

//----------------------------------------------------------------------

void SwgCuiCraftDraft::OnCheckboxSet             (UIWidget * context)
{
	if (context == m_checkShowAll)
		reset ();
}

//----------------------------------------------------------------------

void SwgCuiCraftDraft::OnCheckboxUnset           (UIWidget * context)
{
	if (context == m_checkShowAll)
		reset ();
}

//----------------------------------------------------------------------

UIDataSourceContainer * SwgCuiCraftDraft::populateGameTypeDraftData (const SchematicDataVector & sdv, UIDataSourceContainer & dsc_gameType, const uint32 match_crc)
{
	typedef stdmultimap<Unicode::String, UIDataSourceContainer *>::fwd AlphabetizedDraftData;
	AlphabetizedDraftData add;
	
	UIDataSourceContainer * retval = 0;

	for (SchematicDataVector::const_iterator dit = sdv.begin (); dit != sdv.end (); ++dit)
	{
		const InternalSchematicData & internalSchematicData = *dit;
		
		UIDataSourceContainer * const dsc_draft = new UIDataSourceContainer;
		dsc_draft->SetName            (ObjectTemplateList::lookUp(internalSchematicData.crc.second).getString());
		dsc_draft->SetPropertyLong    (Properties::CrcServer,                 internalSchematicData.crc.first);
		dsc_draft->SetPropertyLong    (Properties::CrcShared,                 internalSchematicData.crc.second);
		dsc_draft->SetProperty        (UITreeView::DataProperties::LocalText, internalSchematicData.displayName);
		dsc_draft->SetPropertyInteger (Properties::Index,                     internalSchematicData.index);
		
		add.insert (std::make_pair (internalSchematicData.displayName, dsc_draft));

		if (match_crc == internalSchematicData.crc.first)
			retval = dsc_draft;
	}
	
	for (AlphabetizedDraftData::const_iterator ait = add.begin (); ait != add.end (); ++ait)
	{
		UIDataSourceContainer * const dsc_draft = (*ait).second;
		dsc_gameType.AddChild (dsc_draft);
	}

	return retval;
}

//======================================================================
