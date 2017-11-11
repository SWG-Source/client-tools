//======================================================================
//
// SwgCuiAuctionFilter.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAuctionFilter.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIDataSourceContainer.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIScrollbar.h"
#include "UITable.h"
#include "UITableHeader.h"
#include "UITableModelDefault.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UITreeView.h"
#include "UnicodeUtils.h"
#include "clientGame/AuctionManagerClient.h"
#include "clientGame/ClientObject.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedGame/CommoditiesAdvancedSearchAttribute.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/ResourceClassObject.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedGame/Universe.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CachedNetworkId.h"

#include <algorithm>
#include <list>

//======================================================================

namespace 
{
	namespace DataProperties
	{
		namespace TypeTree
		{
			const UILowerString Text            = UILowerString ("Text");
			const UILowerString ItemGot         = UILowerString ("ItemGot");
			const UILowerString ItemTemplateCrc = UILowerString ("ItemTemplateCrc");
			const UILowerString ItemIsResourceContainer = UILowerString ("ItemIsResourceContainer");
			const UILowerString ItemIsSpecialGeneralGot = UILowerString ("ItemIsSpecialGeneralGot");
		}
	}

	typedef SwgCuiAuctionFilter::ObjectTypeVector ObjectTypeVector;

	//----------------------------------------------------------------------

	// if the display string is the same, then compare based on the item's crc;
	// this way, same named items will always appear in the same order in the
	// object type filter tree
	struct SubTypeGotNameMapLessCompare
	{
		bool operator()(std::pair<Unicode::String, int> s1, std::pair<Unicode::String, int> s2) const
		{
			const Unicode::String lowerCase1 = Unicode::toLower(s1.first);
			const Unicode::String lowerCase2 = Unicode::toLower(s2.first);

			if (lowerCase1 < lowerCase2)
				return true;
			else if (lowerCase1 == lowerCase2)
				return (s1.second < s2.second);

			return false;
		}
	};

	//----------------------------------------------------------------------

	void updateResourceContainerSubTypeTreeLabels (UIDataSourceContainer & dsc, ResourceClassObject const * rco)
	{
		if (rco)
		{
			std::map<std::pair<Unicode::String, std::string>, ResourceClassObject const *> orderedChildren;
			std::vector<ResourceClassObject const *> children;
			rco->getChildren(children, false);
			for (std::vector<ResourceClassObject const *>::const_iterator iter = children.begin(); iter != children.end(); ++iter)
			{
				if (!ResourceClassObject::isClassExcludedFromCommodities((*iter)->getResourceClassName()))
					orderedChildren[std::make_pair((*iter)->getFriendlyName().localize(), (*iter)->getResourceClassName())] = *iter;
			}
			for (std::map<std::pair<Unicode::String, std::string>, ResourceClassObject const *>::const_iterator iter2 = orderedChildren.begin(); iter2 != orderedChildren.end(); ++iter2)
			{
				int const resourceClassCrc = static_cast<int>(Crc::calculate(iter2->first.second.c_str()));
				UIDataSourceContainer * const dscChild = new UIDataSourceContainer;
				dsc.AddChild(dscChild);
				dscChild->SetName(iter2->first.second);
				dscChild->SetProperty(DataProperties::TypeTree::Text, iter2->first.first);
				dscChild->SetPropertyInteger(DataProperties::TypeTree::ItemGot, SharedObjectTemplate::GOT_resource_container);
				dscChild->SetPropertyInteger(DataProperties::TypeTree::ItemTemplateCrc, resourceClassCrc);

				if (!iter2->second->isLeaf())
				{
					updateResourceContainerSubTypeTreeLabels(*dscChild, iter2->second);
				}
				else
				{
					const AuctionManagerClient::ResourceTypeNameList & resourceTypeNameList = AuctionManagerClient::getResourceTypeNameList(resourceClassCrc);
					for (AuctionManagerClient::ResourceTypeNameList::const_iterator it = resourceTypeNameList.begin(); it != resourceTypeNameList.end(); ++it)
					{
						UIDataSourceContainer * const dscGrandChild = new UIDataSourceContainer;
						dscChild->AddChild(dscGrandChild);
						dscGrandChild->SetName(it->first);
						dscGrandChild->SetProperty(DataProperties::TypeTree::Text, Unicode::narrowToWide(it->first));
						dscGrandChild->SetPropertyInteger(DataProperties::TypeTree::ItemGot, SharedObjectTemplate::GOT_resource_container);
						dscGrandChild->SetPropertyInteger(DataProperties::TypeTree::ItemTemplateCrc, it->second);
						dscGrandChild->SetPropertyInteger(DataProperties::TypeTree::ItemIsResourceContainer, 1);
					}
				}
			}

			DEBUG_WARNING(orderedChildren.empty(), ("resource node %s (%s) added to Object Type Filter tree but no children were added\n", Unicode::wideToNarrow(rco->getFriendlyName().localize()).c_str(), rco->getResourceClassName().c_str()));			
		}
	}

	//----------------------------------------------------------------------

	typedef std::multimap<std::pair<Unicode::String, int>, std::pair<int, std::string>, SubTypeGotNameMapLessCompare> SubTypeGotNameMap;

	//----------------------------------------------------------------------

	void updateSubTypeTreeLabels (UIDataSourceContainer & dsc, int generalGot)
	{
		SubTypeGotNameMap gotNamesMap;
		
		{
			// this is the list of general/masked game object type that
			// should add a node under itself to represent items that
			// is of that general/masked game object type; and yes, we
			// do have items whose game object type is a general/masked
			// game object type and not a subtype game object type
			const bool isSpecialGeneralGot = !CommoditiesAdvancedSearchAttribute::allowChildGameObjectTypeToInheritSearchAttribute(generalGot);

			const GameObjectTypes::TypeStringMap & gotMap = GameObjectTypes::getTypeStringMap ();
			for (GameObjectTypes::TypeStringMap::const_iterator it = gotMap.begin (); it != gotMap.end (); ++it)
			{
				const int got               = (*it).first;

				if (!GameObjectTypes::isTypeOf (got, generalGot))
					continue;

				if (got == generalGot && !isSpecialGeneralGot)
					continue;

				if (GameObjectTypes::isExcludedFromCommodities(got))
					continue;
				
				const int maskedType = GameObjectTypes::getMaskedType (got);
				if (maskedType != got)
				{
					if (GameObjectTypes::isExcludedFromCommodities(maskedType))
						continue;
				}
				
				IGNORE_RETURN(gotNamesMap.insert (std::make_pair (std::make_pair (GameObjectTypes::getLocalizedName (got), 0), std::make_pair (got, GameObjectTypes::getCanonicalName (got)))));
			}

			if (!isSpecialGeneralGot)
			{
				const AuctionManagerClient::ItemTypeNameList & itemTypeNameList = AuctionManagerClient::getItemTypeNameList(generalGot);
				for (AuctionManagerClient::ItemTypeNameList::const_iterator it2 = itemTypeNameList.begin(); it2 != itemTypeNameList.end(); ++it2)
					IGNORE_RETURN(gotNamesMap.insert (std::make_pair (std::make_pair (it2->first, it2->second.first), std::make_pair (generalGot, it2->second.second))));
			}
		}

		for (SubTypeGotNameMap::const_iterator it = gotNamesMap.begin (); it != gotNamesMap.end (); ++it)
		{
			const Unicode::String & localName = (*it).first.first;
			const std::string & canonicalName = (*it).second.second;
			const int                     got = (*it).second.first;
			const int         itemTemplateCrc = (*it).first.second;

			UIDataSourceContainer * const dsc_got = new UIDataSourceContainer;
			dsc.AddChild         (dsc_got);
			dsc_got->SetName     (canonicalName);
			dsc_got->SetProperty (DataProperties::TypeTree::Text, localName);
			dsc_got->SetPropertyInteger (DataProperties::TypeTree::ItemGot, got);

			if (itemTemplateCrc != 0)
			{
				dsc_got->SetPropertyInteger (DataProperties::TypeTree::ItemTemplateCrc, itemTemplateCrc);
			}
			else
			{
				if (got == generalGot)
				{
					dsc_got->SetPropertyInteger(DataProperties::TypeTree::ItemIsSpecialGeneralGot, 1);
				}

				const AuctionManagerClient::ItemTypeNameList & itemTypeNameList = AuctionManagerClient::getItemTypeNameList(got);
				for (AuctionManagerClient::ItemTypeNameList::const_iterator it2 = itemTypeNameList.begin(); it2 != itemTypeNameList.end(); ++it2)
				{
					UIDataSourceContainer * const dsc_got_item = new UIDataSourceContainer;
					dsc_got->AddChild (dsc_got_item);
					dsc_got_item->SetName (it2->second.second);
					dsc_got_item->SetProperty (DataProperties::TypeTree::Text, it2->first);
					dsc_got_item->SetPropertyInteger (DataProperties::TypeTree::ItemGot, got);
					dsc_got_item->SetPropertyInteger (DataProperties::TypeTree::ItemTemplateCrc, it2->second.first);
				}
			}
		}
	}

	//----------------------------------------------------------------------

	typedef std::multimap<Unicode::String, int> GotNameMap;
	
	//----------------------------------------------------------------------

	void updateTypeTreeLabels (UIDataSourceContainer & dsc)
	{
		GotNameMap gotNamesMap;

		{
			const GameObjectTypes::TypeStringMap & gotMap = GameObjectTypes::getTypeStringMap ();
			
			for (GameObjectTypes::TypeStringMap::const_iterator it = gotMap.begin (); it != gotMap.end (); ++it)
			{
				const int got               = (*it).first;
				
				if (GameObjectTypes::isSubType (got))
					continue;

				if (GameObjectTypes::isExcludedFromCommodities(got))
					continue;
				
				IGNORE_RETURN(gotNamesMap.insert (std::make_pair (GameObjectTypes::getLocalizedName (got), got)));
			}
		}

		//-- make the 'none' type come first
		{
			const int               got       = SharedObjectTemplate::GOT_none;
			const Unicode::String & localName = Unicode::narrowToWide ("All");

			UIDataSourceContainer * const dsc_got = new UIDataSourceContainer;
			dsc.AddChild         (dsc_got);
			dsc_got->SetName     (GameObjectTypes::getCanonicalName (got));
			dsc_got->SetProperty (DataProperties::TypeTree::Text, localName);
			dsc_got->SetPropertyInteger (DataProperties::TypeTree::ItemGot, got);
		}

		for (GotNameMap::const_iterator it = gotNamesMap.begin (); it != gotNamesMap.end (); ++it)
		{
			const Unicode::String & localName = (*it).first;
			const int               got       = (*it).second;

			UIDataSourceContainer * const dsc_got = new UIDataSourceContainer;
			dsc.AddChild         (dsc_got);
			dsc_got->SetName     (GameObjectTypes::getCanonicalName (got));
			dsc_got->SetProperty (DataProperties::TypeTree::Text, localName);
			dsc_got->SetPropertyInteger (DataProperties::TypeTree::ItemGot, got);

			if (got && !GameObjectTypes::isSubType (got))
			{
				if (got != SharedObjectTemplate::GOT_resource_container)
				{
					updateSubTypeTreeLabels (*dsc_got, got);
				}
				else
				{
					updateResourceContainerSubTypeTreeLabels (*dsc_got, Universe::getInstance().getResourceTreeRoot());
				}
			}
		}
	}

	//----------------------------------------------------------------------

	std::map<int, std::map<Unicode::String, std::pair<Unicode::String, std::string> > > s_attributeFilterDisplayString;

	std::map<Unicode::String, std::pair<Unicode::String, std::string> > const & getAttributeFilterDisplayString(int got)
	{
		static std::map<Unicode::String, std::pair<Unicode::String, std::string> > const empty;

		std::map<int, std::map<Unicode::String, std::pair<Unicode::String, std::string> > >::const_iterator const iterFind = s_attributeFilterDisplayString.find(got);
		if (iterFind != s_attributeFilterDisplayString.end())
			return iterFind->second;

		return empty;
	}

	//----------------------------------------------------------------------

	std::map<std::pair<int, std::string>, std::map<Unicode::String, std::pair<Unicode::String, std::string> > > s_attributeFilterEnumDisplayString;

	std::map<Unicode::String, std::pair<Unicode::String, std::string> > const & getAttributeFilterEnumDisplayString(int got, std::string const & attributeName)
	{
		static std::map<Unicode::String, std::pair<Unicode::String, std::string> > const empty;

		std::map<std::pair<int, std::string>, std::map<Unicode::String, std::pair<Unicode::String, std::string> > >::const_iterator const iterFind = s_attributeFilterEnumDisplayString.find(std::make_pair(got, attributeName));
		if (iterFind != s_attributeFilterEnumDisplayString.end())
			return iterFind->second;

		return empty;
	}

	//----------------------------------------------------------------------

	void addAttributeFilter(Unicode::String const & filterAttributeDisplayName, Unicode::String const & filterAttributeDisplayValue, bool requiredAttribute, UITable & table, bool selectAddedRow)
	{
		UITableModelDefault * const tableModel = safe_cast<UITableModelDefault *>(table.GetTableModel());
		if (!tableModel)
			return;

		tableModel->AppendCell(0, NULL, filterAttributeDisplayName);
		tableModel->AppendCell(1, NULL, filterAttributeDisplayValue);
		tableModel->AppendCell(2, NULL, (requiredAttribute ? Unicode::narrowToWide("Yes") : Unicode::narrowToWide("No")));

		if (selectAddedRow)
		{
			int const newRowVisualIndex = tableModel->GetVisualDataRowIndex(table.GetRowCount() - 1);
			if (newRowVisualIndex >= 0)
			{
				table.SelectRow(newRowVisualIndex);
				table.ScrollToRow(newRowVisualIndex);
			}
		}
	}

	void updateAttributeFilter(Unicode::String const & filterAttributeDisplayName, Unicode::String const & filterAttributeDisplayValue, bool requiredAttribute, UITable & table, int row)
	{
		if ((row < 0) || (row >= table.GetRowCount()))
			return;

		UITableModelDefault * const tableModel = safe_cast<UITableModelDefault *>(table.GetTableModel());
		if (!tableModel)
			return;

		tableModel->SetValueAtText(row, 0, filterAttributeDisplayName);
		tableModel->SetValueAtText(row, 1, filterAttributeDisplayValue);
		tableModel->SetValueAtText(row, 2, (requiredAttribute ? Unicode::narrowToWide("Yes") : Unicode::narrowToWide("No")));
	}

	//----------------------------------------------------------------------

	std::map<std::pair<int, int>, std::list<AuctionQueryHeadersMessage::SearchCondition> > s_attributeFilter;
	std::map<std::pair<int, int>, std::list<std::pair<Unicode::String, Unicode::String> > > s_attributeFilterDisplayData;
	std::set<std::pair<int, int> > s_enableItemAttributeFilter;
	std::map<std::pair<int, int>, AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny> s_itemAttributeFilterMatchAllAny;

	//----------------------------------------------------------------------

	AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny getItemAttributeFilterMatchAllAny(int objectType, int objectTemplateId)
	{
		std::map<std::pair<int, int>, AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny>::const_iterator iterFind = s_itemAttributeFilterMatchAllAny.find(std::make_pair(objectType, objectTemplateId));
		if (iterFind != s_itemAttributeFilterMatchAllAny.end())
			return iterFind->second;

		return AuctionQueryHeadersMessage::ASMAA_match_all;
	}

	//----------------------------------------------------------------------

	std::set<SwgCuiAuctionFilter *> s_activeFilterWindow;
	std::map<std::pair<int, int>, Unicode::String> s_nameFilter;
	Unicode::String s_minPriceFilter;
	Unicode::String s_maxPriceFilter;
	bool s_includeEntranceFee = false;

	//----------------------------------------------------------------------

	int getComboBoxIndexFromIndexName(UIComboBox const & comboBox, std::string const & indexName)
	{
		std::string currentIndexName;
		for (int i = 0, size = comboBox.GetItemCount(); i < size; ++i)
		{
			if (comboBox.GetIndexName(i, currentIndexName) && (currentIndexName == indexName))
				return i;
		}

		return -1;
	}

	//----------------------------------------------------------------------

	int getComboBoxIndexFromIndexNameCrc(UIComboBox const & comboBox, uint32 const indexNameCrc)
	{
		std::string currentIndexName;
		for (int i = 0, size = comboBox.GetItemCount(); i < size; ++i)
		{
			if (comboBox.GetIndexName(i, currentIndexName) && (Crc::calculate(currentIndexName.c_str()) == indexNameCrc))
				return i;
		}

		return -1;
	}

	//----------------------------------------------------------------------

	// "object_type" crc value
	uint32 const s_objectTypeAttributeNameCrc = 1052401998;
}

//----------------------------------------------------------------------

SwgCuiAuctionFilter::SwgCuiAuctionFilter (UIPage & page, const char * const debugName) :
CuiMediator          (debugName, page),
UIEventCallback      (),
m_callback           (new MessageDispatch::Callback),
m_treeTypes          (0),
m_radioGalaxy        (0),
m_radioPlanet        (0),
m_radioCity          (0),
m_radioMarket        (0),
m_textboxTextFilter  (0),
m_textboxMinPriceFilter(0),
m_textboxMaxPriceFilter(0),
m_includeEntranceFee(0),
m_enableItemAttributeFilter(0),
m_comboboxMatchAllAny(0),
m_attributeFilterTableHeader(0),
m_attributeFilterTable(0),
m_attributeFilterTableScrollbar(0),
m_buttonAddFilterAttribute(0),
m_buttonUpdateFilterAttribute(0),
m_buttonRemoveFilterAttribute(0),
m_buttonRemoveAllFilterAttribute(0),
m_labelAttributeName(0),
m_comboboxAttributeName(0),
m_labelMinValue(0),
m_pageMinIntValue(0),
m_textboxMinIntValue(0),
m_pageMinFloatValue(0),
m_textboxMinFloatValue(0),
m_comboboxStringCompare(0),
m_comboboxEnumCompare(0),
m_labelMaxValue(0),
m_labelValue(0),
m_pageMaxIntValue(0),
m_textboxMaxIntValue(0),
m_pageMaxFloatValue(0),
m_textboxMaxFloatValue(0),
m_pageStringValue(0),
m_textboxStringValue(0),
m_pageComboValue(0),
m_comboboxValue(0),
m_requiredAttribute(0),
m_transceiverChanged (new Transceivers::Changed),
m_requestUpdateNextFrame(false),
m_ignoreFilterChangeRequestUpdate  (false),
m_itemTypeListVersion(),
m_resourceTypeListVersion(),
m_currentlySelectedRowDSC(NULL),
m_checkpointTextFilterAll(),
m_checkpointTextFilterAny(),
m_checkpointPriceFilterMin(0),
m_checkpointPriceFilterMax(0),
m_checkpointPriceFilterIncludesEntranceFee(false),
m_checkpointItemAttributeFilterMatchAllAny(AuctionQueryHeadersMessage::ASMAA_match_all),
m_checkpointAttributeFilter(),
m_currentlySelectedObjectType(SharedObjectTemplate::GOT_none),
m_currentlySelectedObjectTemplateId(0),
m_currentlySelectedObjectIsSpecialGeneralGot(false)
{
	getCodeDataObject (TUITreeView, m_treeTypes,             "treeTypes",         true);
	getCodeDataObject (TUICheckbox, m_radioGalaxy,           "radioGalaxy",       true);
	getCodeDataObject (TUICheckbox, m_radioPlanet,           "radioPlanet",       true);
	getCodeDataObject (TUICheckbox, m_radioCity,             "radioCity",         true);
	getCodeDataObject (TUICheckbox, m_radioMarket,           "radioMarket",       true);
	getCodeDataObject (TUITextbox,  m_textboxTextFilter,     "textboxTextFilter", true);
	getCodeDataObject (TUITextbox,  m_textboxMinPriceFilter, "textboxMinPriceFilter", true);
	getCodeDataObject (TUITextbox,  m_textboxMaxPriceFilter, "textboxMaxPriceFilter", true);
	getCodeDataObject (TUICheckbox, m_includeEntranceFee,    "includeEntranceFee",    true);

	getCodeDataObject (TUICheckbox, m_enableItemAttributeFilter, "includeAttributeFilter", true);

	if (m_enableItemAttributeFilter)
	{
		getCodeDataObject (TUIComboBox, m_comboboxMatchAllAny, "comboMatchAllAny", false);
		getCodeDataObject (TUITableHeader, m_attributeFilterTableHeader, "header", false);
		getCodeDataObject (TUITable, m_attributeFilterTable, "Table", false);
		getCodeDataObject (TUIScrollbar, m_attributeFilterTableScrollbar, "scroll", false);
		getCodeDataObject (TUIButton, m_buttonAddFilterAttribute, "buttonAddFilterAttribute", false);
		getCodeDataObject (TUIButton, m_buttonUpdateFilterAttribute, "buttonUpdateFilterAttribute", false);
		getCodeDataObject (TUIButton, m_buttonRemoveFilterAttribute, "buttonRemoveFilterAttribute", false);
		getCodeDataObject (TUIButton, m_buttonRemoveAllFilterAttribute, "buttonRemoveAllFilterAttribute", false);
		getCodeDataObject (TUIText, m_labelAttributeName, "labelAttributeName", false);
		getCodeDataObject (TUIComboBox, m_comboboxAttributeName, "comboAttributeName", false);
		getCodeDataObject (TUIText, m_labelMinValue, "labelMinValue", false);
		getCodeDataObject (TUIPage, m_pageMinIntValue, "pageMinIntValue", false);
		getCodeDataObject (TUITextbox,  m_textboxMinIntValue, "textboxMinIntValue", false);
		getCodeDataObject (TUIPage, m_pageMinFloatValue, "pageMinFloatValue", false);
		getCodeDataObject (TUITextbox,  m_textboxMinFloatValue, "textboxMinFloatValue", false);
		getCodeDataObject (TUIComboBox, m_comboboxStringCompare, "comboStringCompare", false);
		getCodeDataObject (TUIComboBox, m_comboboxEnumCompare, "comboEnumCompare", false);
		getCodeDataObject (TUIText, m_labelMaxValue, "labelMaxValue", false);
		getCodeDataObject (TUIText, m_labelValue, "labelValue", false);
		getCodeDataObject (TUIPage, m_pageMaxIntValue, "pageMaxIntValue", false);
		getCodeDataObject (TUITextbox,  m_textboxMaxIntValue, "textboxMaxIntValue", false);
		getCodeDataObject (TUIPage, m_pageMaxFloatValue, "pageMaxFloatValue", false);
		getCodeDataObject (TUITextbox,  m_textboxMaxFloatValue, "textboxMaxFloatValue", false);
		getCodeDataObject (TUIPage, m_pageStringValue, "pageStringValue", false);
		getCodeDataObject (TUITextbox,  m_textboxStringValue, "textboxStringValue", false);
		getCodeDataObject (TUIPage, m_pageComboValue, "pageComboValue", false);
		getCodeDataObject (TUIComboBox, m_comboboxValue, "comboValue", false);
		getCodeDataObject (TUICheckbox, m_requiredAttribute, "requiredAttribute", false);
	}

	if(m_textboxTextFilter != 0)
	{
		m_textboxTextFilter->SetText(Unicode::emptyString);
	}

	if(m_textboxMinPriceFilter != 0)
	{
		m_textboxMinPriceFilter->SetMaxIntegerLength(9);
		m_textboxMinPriceFilter->SetText(Unicode::emptyString);
	}

	if(m_textboxMaxPriceFilter != 0)
	{
		m_textboxMaxPriceFilter->SetMaxIntegerLength(9);
		m_textboxMaxPriceFilter->SetText(Unicode::emptyString);
	}

	if (m_treeTypes)
		registerMediatorObject (*m_treeTypes,       true);	
	if (m_radioGalaxy)
		registerMediatorObject (*m_radioGalaxy,     true);
	if (m_radioPlanet)
		registerMediatorObject (*m_radioPlanet,     true);
	if (m_radioCity)
		registerMediatorObject (*m_radioCity,       true);
	if (m_radioMarket)
		registerMediatorObject (*m_radioMarket,     true);
	if (m_includeEntranceFee)
		registerMediatorObject (*m_includeEntranceFee, true);

	if (m_treeTypes)
	{
		UIDataSourceContainer * const dsc = m_treeTypes->GetDataSourceContainer ();
		NOT_NULL (dsc);

		dsc->Attach (0);
		m_treeTypes->SetDataSourceContainer (0);
		dsc->Clear ();
		updateTypeTreeLabels (*dsc);
		m_treeTypes->SetDataSourceContainer (dsc);
		dsc->Detach (0);

		m_treeTypes->SelectRow (0);

		m_itemTypeListVersion = AuctionManagerClient::getItemTypeListVersion ();
		m_resourceTypeListVersion = AuctionManagerClient::getResourceTypeListVersion ();
	}

	if (m_radioCity)
		m_radioCity->SetChecked (true);

	if (m_includeEntranceFee)
		m_includeEntranceFee->SetChecked(false);

	if (m_enableItemAttributeFilter)
	{
		m_enableItemAttributeFilter->SetVisible(false);
		m_enableItemAttributeFilter->SetChecked(false);

		m_comboboxMatchAllAny->SetSelectedIndex(0);

		UITableModelDefault * const tableModel = dynamic_cast<UITableModelDefault *>(m_attributeFilterTable->GetTableModel());
		if (tableModel)
			tableModel->ClearTable();

		m_comboboxAttributeName->Clear();

		m_comboboxStringCompare->SetSelectedIndex(0);
		m_comboboxEnumCompare->SetSelectedIndex(0);

		m_textboxMinIntValue->SetText(Unicode::emptyString);
		m_textboxMinFloatValue->SetText(Unicode::emptyString);
		m_textboxMaxIntValue->SetText(Unicode::emptyString);
		m_textboxMaxFloatValue->SetText(Unicode::emptyString);
		m_textboxStringValue->SetText(Unicode::emptyString);

		m_comboboxValue->Clear();
		m_comboboxValue->SetScrollbarOpacity(0.7f);

		// maximum 128 characters in value combobox text field
		UITextbox * textboxValue = static_cast<UITextbox *> (m_comboboxValue->GetChild("ComboTextbox"));
		if (textboxValue)
		{
			textboxValue->SetProperty(UITextbox::PropertyName::MaxLength, Unicode::narrowToWide("128"));
			textboxValue->SetMaximumCharacters(128);
		}

		enableControlsForItemAttributeFilter(false);

		enableButtonsForItemAttributeFilter();

		registerMediatorObject(*m_enableItemAttributeFilter, true);
		registerMediatorObject(*m_comboboxMatchAllAny, true);
		registerMediatorObject(*m_attributeFilterTable, true);
		registerMediatorObject(*m_comboboxAttributeName, true);
		registerMediatorObject(*m_buttonAddFilterAttribute, true);
		registerMediatorObject(*m_buttonUpdateFilterAttribute, true);
		registerMediatorObject(*m_buttonRemoveFilterAttribute, true);
		registerMediatorObject(*m_buttonRemoveAllFilterAttribute, true);
	}
}

//----------------------------------------------------------------------

SwgCuiAuctionFilter::~SwgCuiAuctionFilter ()
{
	s_activeFilterWindow.erase(this);

	delete m_callback;
	m_callback = 0;

	delete m_transceiverChanged;
	m_transceiverChanged = 0;
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::performActivate ()
{
	m_callback->connect (*this, &SwgCuiAuctionFilter::onItemTypeListChanged, static_cast<AuctionManagerClient::Messages::ItemTypeListChanged*> (0));
	m_callback->connect (*this, &SwgCuiAuctionFilter::onResourceTypeListChanged, static_cast<AuctionManagerClient::Messages::ResourceTypeListChanged*> (0));

	if (m_radioMarket)
	{
		std::string planet;
		std::string region;
		std::string name;
		
		IGNORE_RETURN(AuctionManagerClient::getMarketLocationStrings (planet, region, name));
		m_radioMarket->SetText (Unicode::narrowToWide (name));
	}

	updateFilterTree();

	refreshItemAttributeFilter();
	enableControlsForItemAttributeFilter(false);
	enableButtonsForItemAttributeFilter();

	// tell any currently active filter window (which will soon be deactivated)
	// to save their name price filter information, so we can use it in our window;
	// we need to do this because in some cases, this filter window gets
	// activated before the other one gets deactivated
	for (std::set<SwgCuiAuctionFilter *>::const_iterator iter = s_activeFilterWindow.begin(); iter != s_activeFilterWindow.end(); ++iter)
	{
		if (*iter != this)
		{
			(*iter)->saveNamePriceFilter();
		}
	}

	restoreNamePriceFilter();

	s_activeFilterWindow.insert(this);
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::performDeactivate ()
{
	m_callback->disconnect (*this, &SwgCuiAuctionFilter::onItemTypeListChanged, static_cast<AuctionManagerClient::Messages::ItemTypeListChanged*> (0));
	m_callback->disconnect (*this, &SwgCuiAuctionFilter::onResourceTypeListChanged, static_cast<AuctionManagerClient::Messages::ResourceTypeListChanged*> (0));

	saveNamePriceFilter();

	s_activeFilterWindow.erase(this);
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::OnButtonPressed (UIWidget *context)
{
	if ((context == m_buttonAddFilterAttribute) || (context == m_buttonUpdateFilterAttribute))
	{
		if (context == m_buttonAddFilterAttribute)
		{
			// max 10 item attribute search filters
			if (m_attributeFilterTable->GetRowCount() >= 10)
			{
				CuiMessageBox::createInfoBox(Unicode::narrowToWide("You have reached the limit of 10 filters in the item attribute filter list."));
				return;
			}
		}

		if (m_comboboxAttributeName->GetSelectedIndex() < 0)
			return;

		int selectedRow = -1;
		std::pair<Unicode::String, Unicode::String> * selectedRowData = NULL;
		AuctionQueryHeadersMessage::SearchCondition * selectedFilter = NULL;
		bool factoryCrateObjectTypeFilterBeingUpdated = false;
		if (context == m_buttonUpdateFilterAttribute)
		{
			selectedRow = static_cast<int>(m_attributeFilterTable->GetLastSelectedRow());
			if (selectedRow < 0)
				return;

			UITableModelDefault * const tableModel = safe_cast<UITableModelDefault *>(m_attributeFilterTable->GetTableModel());
			if (!tableModel)
				return;

			int selectedRowLogicalIndex = tableModel->GetLogicalDataRowIndex(selectedRow);
			if (selectedRowLogicalIndex < 0)
				return;

			std::map<std::pair<int, int>, std::list<AuctionQueryHeadersMessage::SearchCondition> >::iterator const iterFind1 = s_attributeFilter.find(std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId));
			if ((iterFind1 == s_attributeFilter.end()) || (static_cast<size_t>(selectedRowLogicalIndex) >= iterFind1->second.size()))
				return;

			std::map<std::pair<int, int>, std::list<std::pair<Unicode::String, Unicode::String> > >::iterator const iterFind2 = s_attributeFilterDisplayData.find(std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId));
			if ((iterFind2 == s_attributeFilterDisplayData.end()) || (iterFind2->second.size() != iterFind1->second.size()))
				return;

			std::list<AuctionQueryHeadersMessage::SearchCondition>::iterator iter1 = iterFind1->second.begin();
			std::list<std::pair<Unicode::String, Unicode::String> >::iterator iter2 = iterFind2->second.begin();
			for (; ((iter1 != iterFind1->second.end()) && (iter2 != iterFind2->second.end())); ++iter1, ++iter2)
			{
				// found filter that corresponds to the selected filter
				if (selectedRowLogicalIndex == 0)
				{
					selectedRowData = &(*iter2);
					selectedFilter = &(*iter1);

					break;
				}

				--selectedRowLogicalIndex;
			}

			if ((selectedRow < 0) || !selectedRowData || !selectedFilter)
				return;

			factoryCrateObjectTypeFilterBeingUpdated = ((m_currentlySelectedObjectType == SharedObjectTemplate::GOT_misc_factory_crate) && (selectedFilter->attributeNameCrc == s_objectTypeAttributeNameCrc));
		}

		std::string selectedAttribute;
		m_comboboxAttributeName->GetSelectedIndexName(selectedAttribute);

		CommoditiesAdvancedSearchAttribute::SearchAttribute const * selectedSearchAttribute = CommoditiesAdvancedSearchAttribute::getSearchAttribute(m_currentlySelectedObjectType, selectedAttribute);
		if (!selectedSearchAttribute)
		{
			// for factory crates, the selected attribute name could be from the item type inside the crate
			if (m_currentlySelectedObjectType == SharedObjectTemplate::GOT_misc_factory_crate)
			{
				// if the factory crate node is selected, the value of the "object_type"
				// search condition (if specified) indicates the item type inside the crate
				if (m_currentlySelectedObjectTemplateId == 0)
				{
					std::map<std::pair<int, int>, std::list<AuctionQueryHeadersMessage::SearchCondition> >::const_iterator const iterFindFactoryCrateSearchAttribute = s_attributeFilter.find(std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId));
					if (iterFindFactoryCrateSearchAttribute != s_attributeFilter.end())
					{
						for (std::list<AuctionQueryHeadersMessage::SearchCondition>::const_iterator iterSearchCondition = iterFindFactoryCrateSearchAttribute->second.begin(); iterSearchCondition != iterFindFactoryCrateSearchAttribute->second.end(); ++iterSearchCondition)
						{
							if ((iterSearchCondition->attributeNameCrc == s_objectTypeAttributeNameCrc) && (iterSearchCondition->comparison == AuctionQueryHeadersMessage::SCC_string_equal))
							{
								if (0 == iterSearchCondition->stringValue.find("@got_n:"))
								{
									selectedSearchAttribute = CommoditiesAdvancedSearchAttribute::getSearchAttribute(GameObjectTypes::getGameObjectType(iterSearchCondition->stringValue.substr(7)), selectedAttribute);
								}
								else
								{
									// value may be aliased
									CommoditiesAdvancedSearchAttribute::SearchAttribute const * const factoryCrateObjectTypeSearchAttribute = CommoditiesAdvancedSearchAttribute::getSearchAttribute(m_currentlySelectedObjectType, "object_type");
									if (factoryCrateObjectTypeSearchAttribute)
									{
										for (std::map<std::string, std::string>::const_iterator iterAlias = factoryCrateObjectTypeSearchAttribute->enumValueAliasList.begin(); iterAlias != factoryCrateObjectTypeSearchAttribute->enumValueAliasList.end(); ++iterAlias)
										{
											if (iterAlias->second == iterSearchCondition->stringValue)
											{
												if (0 == iterAlias->first.find("@got_n:"))
												{
													selectedSearchAttribute = CommoditiesAdvancedSearchAttribute::getSearchAttribute(GameObjectTypes::getGameObjectType(iterAlias->first.substr(7)), selectedAttribute);
												}

												break;
											}
										}
									}
								}

								break;
							}
						}
					}
				}
				// if an item node under the factory node is selected, get the item
				// type value from it which indicates the item type inside the crate
				else
				{
					const AuctionManagerClient::ItemTypeNameListServer & factoryCrateItemTypeNameList = AuctionManagerClient::getItemTypeNameListServer(m_currentlySelectedObjectType);
					AuctionManagerClient::ItemTypeNameListServer::const_iterator const iterFindTemplate = factoryCrateItemTypeNameList.find(m_currentlySelectedObjectTemplateId);
					if ((iterFindTemplate != factoryCrateItemTypeNameList.end()) && (iterFindTemplate->second.first != SharedObjectTemplate::GOT_misc_factory_crate))
					{
						selectedSearchAttribute = CommoditiesAdvancedSearchAttribute::getSearchAttribute(iterFindTemplate->second.first, selectedAttribute);
					}
				}
			}

			if (!selectedSearchAttribute)
				return;
		}

		Unicode::String selectedAttributeDisplay;
		m_comboboxAttributeName->GetSelectedIndexLocalText(selectedAttributeDisplay);

		if (selectedSearchAttribute->attributeDataType == CommoditiesAdvancedSearchAttribute::SADT_int)
		{
			int intMin = 0;
			int intMax = 0;

			Unicode::String valueMin = Unicode::getTrim(m_textboxMinIntValue->GetLocalText());
			Unicode::String valueMax = Unicode::getTrim(m_textboxMaxIntValue->GetLocalText());

			if (valueMin.empty() && valueMax.empty())
			{
				CuiMessageBox::createInfoBox(Unicode::narrowToWide("Please specify a search value."));
				return;
			}

			if (valueMin.empty())
				intMin = -9999999;
			else
				intMin = ::atoi(Unicode::wideToNarrow(valueMin).c_str());

			
			if (valueMax.empty())
				intMax = 9999999;
			else
				intMax = ::atoi(Unicode::wideToNarrow(valueMax).c_str());

			if (intMax < intMin)
				std::swap(intMin, intMax);

			char buffer[128];
			snprintf(buffer, sizeof(buffer)-1, "%d - %d", intMin, intMax);
			buffer[sizeof(buffer)-1] = '\0';

			AuctionQueryHeadersMessage::SearchCondition const filter(selectedSearchAttribute->attributeNameCrc, m_requiredAttribute->IsChecked(), intMin, intMax);

			if (context == m_buttonUpdateFilterAttribute)
			{
				updateAttributeFilter(selectedAttributeDisplay, Unicode::narrowToWide(buffer), filter.requiredAttribute, *m_attributeFilterTable, selectedRow);
				*selectedFilter = filter;
				selectedRowData->first = selectedAttributeDisplay;
				selectedRowData->second = Unicode::narrowToWide(buffer);
			}
			else
			{
				addAttributeFilter(selectedAttributeDisplay, Unicode::narrowToWide(buffer), filter.requiredAttribute, *m_attributeFilterTable, true);
				s_attributeFilter[std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId)].push_back(filter);
				s_attributeFilterDisplayData[std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId)].push_back(std::make_pair(selectedAttributeDisplay, Unicode::narrowToWide(buffer)));
			}
		}
		else if (selectedSearchAttribute->attributeDataType == CommoditiesAdvancedSearchAttribute::SADT_float)
		{
			double floatMin = 0.0f;
			double floatMax = 0.0f;

			Unicode::String valueMin = Unicode::getTrim(m_textboxMinFloatValue->GetLocalText());
			Unicode::String valueMax = Unicode::getTrim(m_textboxMaxFloatValue->GetLocalText());

			if (valueMin.empty() && valueMax.empty())
			{
				CuiMessageBox::createInfoBox(Unicode::narrowToWide("Please specify a search value."));
				return;
			}

			if (valueMin.empty())
				floatMin = -9999999.00f;
			else
				floatMin = ::atof(Unicode::wideToNarrow(valueMin).c_str());

			if (valueMax.empty())
				floatMax = 9999999.00f;
			else
				floatMax = ::atof(Unicode::wideToNarrow(valueMax).c_str());

			if (floatMax < floatMin)
				std::swap(floatMin, floatMax);

			char bufferMin[128];
			int len = snprintf(bufferMin, sizeof(bufferMin)-1, "%.6f", floatMin);
			bufferMin[sizeof(bufferMin)-1] = '\0';

			// trim trailing 0s after the decimal point
			char * pChar = bufferMin + len - 1;
			while (pChar >= bufferMin)
			{
				if (*pChar == '.')
				{
					*pChar = '\0';
					break;
				}
				else if (*pChar != '0')
				{
					++pChar;
					*pChar = '\0';
					break;
				}

				--pChar;
			}

			char bufferMax[128];
			len = snprintf(bufferMax, sizeof(bufferMax)-1, "%.6f", floatMax);
			bufferMax[sizeof(bufferMax)-1] = '\0';

			// trim trailing 0s after the decimal point
			pChar = bufferMax + len - 1;
			while (pChar >= bufferMax)
			{
				if (*pChar == '.')
				{
					*pChar = '\0';
					break;
				}
				else if (*pChar != '0')
				{
					++pChar;
					*pChar = '\0';
					break;
				}

				--pChar;
			}

			char buffer[128];
			snprintf(buffer, sizeof(buffer)-1, "%s - %s", bufferMin, bufferMax);
			buffer[sizeof(buffer)-1] = '\0';

			AuctionQueryHeadersMessage::SearchCondition const filter(selectedSearchAttribute->attributeNameCrc, m_requiredAttribute->IsChecked(), floatMin, floatMax);

			if (context == m_buttonUpdateFilterAttribute)
			{
				updateAttributeFilter(selectedAttributeDisplay, Unicode::narrowToWide(buffer), filter.requiredAttribute, *m_attributeFilterTable, selectedRow);
				*selectedFilter = filter;
				selectedRowData->first = selectedAttributeDisplay;
				selectedRowData->second = Unicode::narrowToWide(buffer);
			}
			else
			{
				addAttributeFilter(selectedAttributeDisplay, Unicode::narrowToWide(buffer), filter.requiredAttribute, *m_attributeFilterTable, true);
				s_attributeFilter[std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId)].push_back(filter);
				s_attributeFilterDisplayData[std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId)].push_back(std::make_pair(selectedAttributeDisplay, Unicode::narrowToWide(buffer)));
			}
		}
		else if (selectedSearchAttribute->attributeDataType == CommoditiesAdvancedSearchAttribute::SADT_string)
		{
			Unicode::String value;

			if (selectedSearchAttribute->defaultSearchValueList.empty())
			{
				value = m_textboxStringValue->GetLocalText();
			}
			else
			{
				m_comboboxValue->GetProperty(UIComboBox::PropertyName::SelectedText, value);
			}

			value = Unicode::getTrim(value);
			if (value.empty())
			{
				CuiMessageBox::createInfoBox(Unicode::narrowToWide("Please specify a search value."));
				return;
			}

			std::string compareStr;
			m_comboboxStringCompare->GetSelectedIndexName(compareStr);

			AuctionQueryHeadersMessage::SearchConditionComparison compare = AuctionQueryHeadersMessage::SCC_string_equal;
			if (compareStr == std::string("contain"))
				compare = AuctionQueryHeadersMessage::SCC_string_contain;
			else if (compareStr == std::string("notContain"))
				compare = AuctionQueryHeadersMessage::SCC_string_not_contain;
			else if (compareStr == std::string("equal"))
				compare = AuctionQueryHeadersMessage::SCC_string_equal;
			else if (compareStr == std::string("notEqual"))
				compare = AuctionQueryHeadersMessage::SCC_string_not_equal;
			else
				return;

			Unicode::String compareDisplayStr;
			m_comboboxStringCompare->GetSelectedIndexLocalText(compareDisplayStr);
			if (compareDisplayStr.empty())
				return;

			compareDisplayStr += Unicode::narrowToWide(" \"");
			compareDisplayStr += value;
			compareDisplayStr += Unicode::narrowToWide("\"");

			AuctionQueryHeadersMessage::SearchCondition const filter(selectedSearchAttribute->attributeNameCrc, m_requiredAttribute->IsChecked(), compare, Unicode::wideToNarrow(value));

			if (context == m_buttonUpdateFilterAttribute)
			{
				updateAttributeFilter(selectedAttributeDisplay, compareDisplayStr, filter.requiredAttribute, *m_attributeFilterTable, selectedRow);
				*selectedFilter = filter;
				selectedRowData->first = selectedAttributeDisplay;
				selectedRowData->second = compareDisplayStr;
			}
			else
			{
				addAttributeFilter(selectedAttributeDisplay, compareDisplayStr, filter.requiredAttribute, *m_attributeFilterTable, true);
				s_attributeFilter[std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId)].push_back(filter);
				s_attributeFilterDisplayData[std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId)].push_back(std::make_pair(selectedAttributeDisplay, compareDisplayStr));
			}
		}
		else if (selectedSearchAttribute->attributeDataType == CommoditiesAdvancedSearchAttribute::SADT_enum)
		{
			Unicode::String value;
			m_comboboxValue->GetSelectedIndexLocalText(value);

			value = Unicode::getTrim(value);
			if (value.empty())
			{
				CuiMessageBox::createInfoBox(Unicode::narrowToWide("Please specify a search value."));
				return;
			}

			std::string valueInternal;
			m_comboboxValue->GetSelectedIndexName(valueInternal);
			if (valueInternal.empty())
				return;

			std::string compareStr;
			m_comboboxEnumCompare->GetSelectedIndexName(compareStr);

			AuctionQueryHeadersMessage::SearchConditionComparison compare = AuctionQueryHeadersMessage::SCC_string_equal;
			if (compareStr == std::string("equal"))
				compare = AuctionQueryHeadersMessage::SCC_string_equal;
			else if (compareStr == std::string("notEqual"))
				compare = AuctionQueryHeadersMessage::SCC_string_not_equal;
			else
				return;

			Unicode::String compareDisplayStr;
			m_comboboxEnumCompare->GetSelectedIndexLocalText(compareDisplayStr);
			if (compareDisplayStr.empty())
				return;

			compareDisplayStr += Unicode::narrowToWide(" \"");
			compareDisplayStr += value;
			compareDisplayStr += Unicode::narrowToWide("\"");

			// for factory crate, can only have at most 1 "object_type equal xxxxx" filter
			if ((m_currentlySelectedObjectType == SharedObjectTemplate::GOT_misc_factory_crate) && (selectedSearchAttribute->attributeNameCrc == s_objectTypeAttributeNameCrc) && (compare == AuctionQueryHeadersMessage::SCC_string_equal))
			{
				std::map<std::pair<int, int>, std::list<AuctionQueryHeadersMessage::SearchCondition> >::const_iterator const iterFindFactoryCrateSearchAttribute = s_attributeFilter.find(std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId));
				if (iterFindFactoryCrateSearchAttribute != s_attributeFilter.end())
				{
					for (std::list<AuctionQueryHeadersMessage::SearchCondition>::const_iterator iterSearchCondition = iterFindFactoryCrateSearchAttribute->second.begin(); iterSearchCondition != iterFindFactoryCrateSearchAttribute->second.end(); ++iterSearchCondition)
					{
						if ((iterSearchCondition->attributeNameCrc == s_objectTypeAttributeNameCrc) && (iterSearchCondition->comparison == AuctionQueryHeadersMessage::SCC_string_equal))
						{
							if ((selectedFilter == NULL) || (selectedFilter != &(*iterSearchCondition)))
							{
								CuiMessageBox::createInfoBox(Unicode::narrowToWide("For Factory Crate, you can only specify the \"Type Equal\" filter condition once."));
								return;
							}
						}
					}
				}
			}

			AuctionQueryHeadersMessage::SearchCondition const filter(selectedSearchAttribute->attributeNameCrc, m_requiredAttribute->IsChecked(), compare, valueInternal);

			if (context == m_buttonUpdateFilterAttribute)
			{
				updateAttributeFilter(selectedAttributeDisplay, compareDisplayStr, filter.requiredAttribute, *m_attributeFilterTable, selectedRow);
				*selectedFilter = filter;
				selectedRowData->first = selectedAttributeDisplay;
				selectedRowData->second = compareDisplayStr;
			}
			else
			{
				addAttributeFilter(selectedAttributeDisplay, compareDisplayStr, filter.requiredAttribute, *m_attributeFilterTable, true);
				s_attributeFilter[std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId)].push_back(filter);
				s_attributeFilterDisplayData[std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId)].push_back(std::make_pair(selectedAttributeDisplay, compareDisplayStr));
			}
		}

		// if the factory crate node is selected, and the "object_type" search attribute was
		// added/updated, repopulate the attribute name list to include search attributes based
		// on the type of item inside the crates, as specified by the value of the "object_type"
		// search attribute, if specified
		if ((m_currentlySelectedObjectType == SharedObjectTemplate::GOT_misc_factory_crate) && (m_currentlySelectedObjectTemplateId == 0) && (factoryCrateObjectTypeFilterBeingUpdated || (selectedSearchAttribute->attributeNameCrc == s_objectTypeAttributeNameCrc)))
		{
			std::map<Unicode::String, std::pair<Unicode::String, std::string> > const * additionalSearchAttribute = NULL;
			std::map<std::pair<int, int>, std::list<AuctionQueryHeadersMessage::SearchCondition> >::const_iterator const iterFindFactoryCrateSearchAttribute = s_attributeFilter.find(std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId));
			if (iterFindFactoryCrateSearchAttribute != s_attributeFilter.end())
			{
				for (std::list<AuctionQueryHeadersMessage::SearchCondition>::const_iterator iterSearchCondition = iterFindFactoryCrateSearchAttribute->second.begin(); iterSearchCondition != iterFindFactoryCrateSearchAttribute->second.end(); ++iterSearchCondition)
				{
					if ((iterSearchCondition->attributeNameCrc == s_objectTypeAttributeNameCrc) && (iterSearchCondition->comparison == AuctionQueryHeadersMessage::SCC_string_equal))
					{
						if (0 == iterSearchCondition->stringValue.find("@got_n:"))
						{
							additionalSearchAttribute = &getAttributeFilterDisplayString(GameObjectTypes::getGameObjectType(iterSearchCondition->stringValue.substr(7)));
						}
						else
						{
							// value may be aliased
							CommoditiesAdvancedSearchAttribute::SearchAttribute const * const factoryCrateObjectTypeSearchAttribute = CommoditiesAdvancedSearchAttribute::getSearchAttribute(m_currentlySelectedObjectType, "object_type");
							if (factoryCrateObjectTypeSearchAttribute)
							{
								for (std::map<std::string, std::string>::const_iterator iterAlias = factoryCrateObjectTypeSearchAttribute->enumValueAliasList.begin(); iterAlias != factoryCrateObjectTypeSearchAttribute->enumValueAliasList.end(); ++iterAlias)
								{
									if (iterAlias->second == iterSearchCondition->stringValue)
									{
										if (0 == iterAlias->first.find("@got_n:"))
										{
											additionalSearchAttribute = &getAttributeFilterDisplayString(GameObjectTypes::getGameObjectType(iterAlias->first.substr(7)));
										}

										break;
									}
								}
							}
						}

						break;
					}
				}
			}

			std::map<Unicode::String, std::pair<Unicode::String, std::string> > const & searchAttribute = getAttributeFilterDisplayString(m_currentlySelectedObjectType);
			populateSearchableAttributeNameCombo(searchAttribute, additionalSearchAttribute, (((m_currentlySelectedObjectType == SharedObjectTemplate::GOT_misc_factory_crate) && (m_currentlySelectedObjectTemplateId != 0)) ? "object_type" : ""));
		}

		enableButtonsForItemAttributeFilter();
	}
	else if (context == m_buttonRemoveFilterAttribute)
	{
		int selectedRow = static_cast<int>(m_attributeFilterTable->GetLastSelectedRow());
		if (selectedRow < 0)
			return;

		UITableModelDefault * const tableModel = safe_cast<UITableModelDefault *>(m_attributeFilterTable->GetTableModel());
		if (!tableModel)
			return;

		int selectedRowLogicalIndex = tableModel->GetLogicalDataRowIndex(selectedRow);
		if (selectedRowLogicalIndex < 0)
			return;

		std::map<std::pair<int, int>, std::list<AuctionQueryHeadersMessage::SearchCondition> >::iterator const iterFind1 = s_attributeFilter.find(std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId));
		if ((iterFind1 == s_attributeFilter.end()) || (static_cast<size_t>(selectedRowLogicalIndex) >= iterFind1->second.size()))
			return;

		std::map<std::pair<int, int>, std::list<std::pair<Unicode::String, Unicode::String> > >::iterator const iterFind2 = s_attributeFilterDisplayData.find(std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId));
		if ((iterFind2 == s_attributeFilterDisplayData.end()) || (iterFind2->second.size() != iterFind1->second.size()))
			return;

		std::list<AuctionQueryHeadersMessage::SearchCondition>::iterator iter1 = iterFind1->second.begin();
		std::list<std::pair<Unicode::String, Unicode::String> >::iterator iter2 = iterFind2->second.begin();
		for (; ((iter1 != iterFind1->second.end()) && (iter2 != iterFind2->second.end())); ++iter1, ++iter2)
		{
			// found filter that corresponds to the selected filter
			if (selectedRowLogicalIndex == 0)
			{
				// for factory crates, include search attributes based on the type of item inside the crates,
				// as specified by the value of the "object_type" search attribute, if specified
				bool const repopulateSearchableAttributeNameCombo = ((m_currentlySelectedObjectType == SharedObjectTemplate::GOT_misc_factory_crate) && (m_currentlySelectedObjectTemplateId == 0) && (iter1->attributeNameCrc == s_objectTypeAttributeNameCrc) && (iter1->comparison == AuctionQueryHeadersMessage::SCC_string_equal));

				// remove the selected filter
				iterFind1->second.erase(iter1);
				iterFind2->second.erase(iter2);

				// repopulate table with the updated filter list
				tableModel->ClearTable();
				m_attributeFilterTable->RemoveRowSelection(selectedRow);

				if (iterFind1->second.empty() || iterFind2->second.empty())
				{
					s_attributeFilter.erase(iterFind1);
					s_attributeFilterDisplayData.erase(iterFind2);
				}
				else
				{
					iter1 = iterFind1->second.begin();
					iter2 = iterFind2->second.begin();
					for (; ((iter1 != iterFind1->second.end()) && (iter2 != iterFind2->second.end())); ++iter1, ++iter2)
					{
						addAttributeFilter(iter2->first, iter2->second, iter1->requiredAttribute, *m_attributeFilterTable, false);
					}

					if (selectedRow >= m_attributeFilterTable->GetRowCount())
						selectedRow = m_attributeFilterTable->GetRowCount() - 1;

					if (selectedRow >= 0)
					{
						m_attributeFilterTable->SelectRow(selectedRow);
						m_attributeFilterTable->ScrollToRow(selectedRow);
					}
				}

				// for factory crates, include search attributes based on the type of item inside the crates,
				// as specified by the value of the "object_type" search attribute, if specified
				if (repopulateSearchableAttributeNameCombo)
				{
					std::map<Unicode::String, std::pair<Unicode::String, std::string> > const & searchAttribute = getAttributeFilterDisplayString(m_currentlySelectedObjectType);
					populateSearchableAttributeNameCombo(searchAttribute, NULL, (((m_currentlySelectedObjectType == SharedObjectTemplate::GOT_misc_factory_crate) && (m_currentlySelectedObjectTemplateId != 0)) ? "object_type" : ""));
				}

				break;
			}

			--selectedRowLogicalIndex;
		}

		enableButtonsForItemAttributeFilter();
	}
	else if (context == m_buttonRemoveAllFilterAttribute)
	{
		UITableModelDefault * const tableModel = safe_cast<UITableModelDefault *>(m_attributeFilterTable->GetTableModel());
		if (!tableModel)
			return;

		// for factory crates, include search attributes based on the type of item inside the crates,
		// as specified by the value of the "object_type" search attribute, if specified
		bool repopulateSearchableAttributeNameCombo = false;

		std::map<std::pair<int, int>, std::list<AuctionQueryHeadersMessage::SearchCondition> >::iterator const iterFind = s_attributeFilter.find(std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId));
		if (iterFind != s_attributeFilter.end())
		{
			// for factory crates, include search attributes based on the type of item inside the crates,
			// as specified by the value of the "object_type" search attribute, if specified
			if ((m_currentlySelectedObjectType == SharedObjectTemplate::GOT_misc_factory_crate) && (m_currentlySelectedObjectTemplateId == 0))
			{
				for (std::list<AuctionQueryHeadersMessage::SearchCondition>::const_iterator iterSearchCondition = iterFind->second.begin(); iterSearchCondition != iterFind->second.end(); ++iterSearchCondition)
				{
					if ((iterSearchCondition->attributeNameCrc == s_objectTypeAttributeNameCrc) && (iterSearchCondition->comparison == AuctionQueryHeadersMessage::SCC_string_equal))
					{
						repopulateSearchableAttributeNameCombo = true;
						break;
					}
				}
			}

			s_attributeFilter.erase(iterFind);
		}

		s_attributeFilterDisplayData.erase(std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId));

		tableModel->ClearTable();

		long const selectedRow = m_attributeFilterTable->GetLastSelectedRow();
		if (selectedRow >= 0)
			m_attributeFilterTable->RemoveRowSelection(selectedRow);

		// for factory crates, include search attributes based on the type of item inside the crates,
		// as specified by the value of the "object_type" search attribute, if specified
		if (repopulateSearchableAttributeNameCombo)
		{
			std::map<Unicode::String, std::pair<Unicode::String, std::string> > const & searchAttribute = getAttributeFilterDisplayString(m_currentlySelectedObjectType);
			populateSearchableAttributeNameCombo(searchAttribute, NULL, (((m_currentlySelectedObjectType == SharedObjectTemplate::GOT_misc_factory_crate) && (m_currentlySelectedObjectTemplateId != 0)) ? "object_type" : ""));
		}

		enableButtonsForItemAttributeFilter();
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::OnCheckboxSet (UIWidget * context)
{
	if (context == m_radioGalaxy || context == m_radioPlanet || context == m_radioCity || context == m_radioMarket)
	{
		if (!m_ignoreFilterChangeRequestUpdate)
			requestUpdate ();
	}
	else if (context == m_enableItemAttributeFilter)
	{
		enableControlsForItemAttributeFilter(false);

		// save off the item attribute filter enabled/disabled state for the selected
		// node, so when when it is selected again, we can restore the state
		int selectedObjectTemplateId;
		bool selectedObjectIsResourceContainer;
		bool selectedObjectIsSpecialGeneralGot;
		int const selectedObjectType = getSelectedObjectType(selectedObjectTemplateId, selectedObjectIsResourceContainer, selectedObjectIsSpecialGeneralGot);

		s_enableItemAttributeFilter.insert(std::make_pair(selectedObjectType, selectedObjectTemplateId));
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::OnCheckboxUnset (UIWidget * context)
{
	if (context == m_enableItemAttributeFilter)
	{
		enableControlsForItemAttributeFilter(false);

		// save off the item attribute filter enabled/disabled state for the selected
		// node, so when when it is selected again, we can restore the state
		int selectedObjectTemplateId;
		bool selectedObjectIsResourceContainer;
		bool selectedObjectIsSpecialGeneralGot;
		int const selectedObjectType = getSelectedObjectType(selectedObjectTemplateId, selectedObjectIsResourceContainer, selectedObjectIsSpecialGeneralGot);

		s_enableItemAttributeFilter.erase(std::make_pair(selectedObjectType, selectedObjectTemplateId));
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::OnGenericSelectionChanged (UIWidget * context)
{
	if (context == m_treeTypes)
	{
		if (!m_ignoreFilterChangeRequestUpdate)
		{
			const long selectedRow = m_treeTypes->GetLastSelectedRow ();
			const UIDataSourceContainer * const dsc = m_treeTypes->GetDataSourceContainerAtRow  (selectedRow);

			if (m_currentlySelectedRowDSC != static_cast<void *>(const_cast<UIDataSourceContainer *>(dsc)))
			{
				m_currentlySelectedRowDSC = static_cast<void *>(const_cast<UIDataSourceContainer *>(dsc));
				requestUpdate ();
			}
		}

		int newSelectedObjectTemplateId;
		bool selectedObjectIsResourceContainer;
		bool newSelectedObjectIsSpecialGeneralGot;
		int const newSelectedObjectType = getSelectedObjectType(newSelectedObjectTemplateId, selectedObjectIsResourceContainer, newSelectedObjectIsSpecialGeneralGot);
		bool const newNodeSelected = ((newSelectedObjectType != m_currentlySelectedObjectType) || (newSelectedObjectTemplateId != m_currentlySelectedObjectTemplateId) || (newSelectedObjectIsSpecialGeneralGot != m_currentlySelectedObjectIsSpecialGeneralGot));

		// does selected object type support item attribute filter?
		if (m_enableItemAttributeFilter && newNodeSelected)
		{
			std::map<Unicode::String, std::pair<Unicode::String, std::string> > const & searchAttribute = getAttributeFilterDisplayString(newSelectedObjectType);
			if (searchAttribute.empty() || (!CommoditiesAdvancedSearchAttribute::allowChildGameObjectTypeToInheritSearchAttribute(newSelectedObjectType) && !newSelectedObjectIsSpecialGeneralGot && (newSelectedObjectTemplateId == 0)))
			{
				m_enableItemAttributeFilter->SetVisible(false);

				if (searchAttribute.empty())
				{
					s_enableItemAttributeFilter.erase(std::make_pair(newSelectedObjectType, newSelectedObjectTemplateId));
				}
			}
			else
			{
				m_enableItemAttributeFilter->SetVisible(true);

				// populate attribute name combo with the searchable attributes

				// for factory crates, include search attributes based on the type of item inside the crates,
				// as specified by the value of the "object_type" search attribute, if specified
				std::map<Unicode::String, std::pair<Unicode::String, std::string> > const * additionalSearchAttribute = NULL;
				if (newSelectedObjectType == SharedObjectTemplate::GOT_misc_factory_crate)
				{
					// if the factory crate node is selected, the value of the "object_type"
					// search condition (if specified) indicates the item type inside the crate
					if (newSelectedObjectTemplateId == 0)
					{
						std::map<std::pair<int, int>, std::list<AuctionQueryHeadersMessage::SearchCondition> >::const_iterator const iterFindFactoryCrateSearchAttribute = s_attributeFilter.find(std::make_pair(newSelectedObjectType, newSelectedObjectTemplateId));
						if (iterFindFactoryCrateSearchAttribute != s_attributeFilter.end())
						{
							for (std::list<AuctionQueryHeadersMessage::SearchCondition>::const_iterator iterSearchCondition = iterFindFactoryCrateSearchAttribute->second.begin(); iterSearchCondition != iterFindFactoryCrateSearchAttribute->second.end(); ++iterSearchCondition)
							{
								if ((iterSearchCondition->attributeNameCrc == s_objectTypeAttributeNameCrc) && (iterSearchCondition->comparison == AuctionQueryHeadersMessage::SCC_string_equal))
								{
									if (0 == iterSearchCondition->stringValue.find("@got_n:"))
									{
										additionalSearchAttribute = &getAttributeFilterDisplayString(GameObjectTypes::getGameObjectType(iterSearchCondition->stringValue.substr(7)));
									}
									else
									{
										// value may be aliased
										CommoditiesAdvancedSearchAttribute::SearchAttribute const * const factoryCrateObjectTypeSearchAttribute = CommoditiesAdvancedSearchAttribute::getSearchAttribute(newSelectedObjectType, "object_type");
										if (factoryCrateObjectTypeSearchAttribute)
										{
											for (std::map<std::string, std::string>::const_iterator iterAlias = factoryCrateObjectTypeSearchAttribute->enumValueAliasList.begin(); iterAlias != factoryCrateObjectTypeSearchAttribute->enumValueAliasList.end(); ++iterAlias)
											{
												if (iterAlias->second == iterSearchCondition->stringValue)
												{
													if (0 == iterAlias->first.find("@got_n:"))
													{
														additionalSearchAttribute = &getAttributeFilterDisplayString(GameObjectTypes::getGameObjectType(iterAlias->first.substr(7)));
													}

													break;
												}
											}
										}
									}

									break;
								}
							}
						}
					}
					// if an item node under the factory node is selected, get the item
					// type value from it which indicates the item type inside the crate
					else
					{
						const AuctionManagerClient::ItemTypeNameListServer & factoryCrateItemTypeNameList = AuctionManagerClient::getItemTypeNameListServer(newSelectedObjectType);
						AuctionManagerClient::ItemTypeNameListServer::const_iterator const iterFindTemplate = factoryCrateItemTypeNameList.find(newSelectedObjectTemplateId);
						if (iterFindTemplate != factoryCrateItemTypeNameList.end() && (iterFindTemplate->second.first != SharedObjectTemplate::GOT_misc_factory_crate))
						{
							additionalSearchAttribute = &getAttributeFilterDisplayString(iterFindTemplate->second.first);
						}
					}
				}

				populateSearchableAttributeNameCombo(searchAttribute, additionalSearchAttribute, (((newSelectedObjectType == SharedObjectTemplate::GOT_misc_factory_crate) && (newSelectedObjectTemplateId != 0)) ? "object_type" : ""));
			}

			refreshItemAttributeFilter();
			enableControlsForItemAttributeFilter(false);
			enableButtonsForItemAttributeFilter();
		}

		if (m_textboxTextFilter && newNodeSelected)
		{
			s_nameFilter[std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId)] = m_textboxTextFilter->GetLocalText();

			std::map<std::pair<int, int>, Unicode::String>::const_iterator const iterFind = s_nameFilter.find(std::make_pair(newSelectedObjectType, newSelectedObjectTemplateId));
			if (iterFind != s_nameFilter.end())
			{
				m_textboxTextFilter->SetText(iterFind->second);
			}
			else
			{
				m_textboxTextFilter->SetText(Unicode::emptyString);
			}
		}

		m_currentlySelectedObjectType = newSelectedObjectType;
		m_currentlySelectedObjectTemplateId = newSelectedObjectTemplateId;
		m_currentlySelectedObjectIsSpecialGeneralGot = newSelectedObjectIsSpecialGeneralGot;
	}
	else if (context == m_comboboxAttributeName)
	{
		enableControlsForItemAttributeFilter(true);
		enableButtonsForItemAttributeFilter();
	}
	else if (context == m_comboboxMatchAllAny)
	{
		// save off the match all/any attribute filter state for the selected
		// node, so when when it is selected again, we can restore the state
		int selectedObjectTemplateId;
		bool selectedObjectIsResourceContainer;
		bool selectedObjectIsSpecialGeneralGot;
		int const selectedObjectType = getSelectedObjectType(selectedObjectTemplateId, selectedObjectIsResourceContainer, selectedObjectIsSpecialGeneralGot);

		s_itemAttributeFilterMatchAllAny[std::make_pair(selectedObjectType, selectedObjectTemplateId)] = static_cast<AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny>(std::max(0L, m_comboboxMatchAllAny->GetSelectedIndex()));
	}
	else if (context == m_attributeFilterTable)
	{
		// update the filter edit fields with the filter values in the selected filter
		int const selectedRow = static_cast<int>(m_attributeFilterTable->GetLastSelectedRow());
		if (selectedRow < 0)
			return;

		UITableModelDefault const * const tableModel = safe_cast<UITableModelDefault const *>(m_attributeFilterTable->GetTableModel());
		if (!tableModel)
			return;

		int selectedRowLogicalIndex = tableModel->GetLogicalDataRowIndex(selectedRow);
		if (selectedRowLogicalIndex < 0)
			return;
			
		std::map<std::pair<int, int>, std::list<AuctionQueryHeadersMessage::SearchCondition> >::const_iterator const iterFind = s_attributeFilter.find(std::make_pair(m_currentlySelectedObjectType, m_currentlySelectedObjectTemplateId));
		if ((iterFind == s_attributeFilter.end()) || (static_cast<size_t>(selectedRowLogicalIndex) >= iterFind->second.size()))
			return;

		for (std::list<AuctionQueryHeadersMessage::SearchCondition>::const_iterator iter = iterFind->second.begin(); iter != iterFind->second.end(); ++iter)
		{
			// found filter that corresponds to the selected filter
			if (selectedRowLogicalIndex == 0)
			{
				int const attributeNameIndex = getComboBoxIndexFromIndexNameCrc(*m_comboboxAttributeName, iter->attributeNameCrc);
				if (attributeNameIndex < 0)
					break;

				m_comboboxAttributeName->SetSelectedIndex(attributeNameIndex);
				m_requiredAttribute->SetChecked(iter->requiredAttribute);

				if (iter->comparison == AuctionQueryHeadersMessage::SCC_int)
				{
					char buffer[128];
					snprintf(buffer, sizeof(buffer)-1, "%d", iter->intMin);
					buffer[sizeof(buffer)-1] = '\0';

					m_textboxMinIntValue->SetText(Unicode::narrowToWide(buffer));

					snprintf(buffer, sizeof(buffer)-1, "%d", iter->intMax);
					buffer[sizeof(buffer)-1] = '\0';

					m_textboxMaxIntValue->SetText(Unicode::narrowToWide(buffer));
				}
				else if (iter->comparison == AuctionQueryHeadersMessage::SCC_float)
				{
					char buffer[128];
					int len = snprintf(buffer, sizeof(buffer)-1, "%.6f", iter->floatMin);
					buffer[sizeof(buffer)-1] = '\0';

					// trim trailing 0s after the decimal point
					char * pChar = buffer + len - 1;
					while (pChar >= buffer)
					{
						if (*pChar == '.')
						{
							*pChar = '\0';
							break;
						}
						else if (*pChar != '0')
						{
							++pChar;
							*pChar = '\0';
							break;
						}

						--pChar;
					}

					m_textboxMinFloatValue->SetText(Unicode::narrowToWide(buffer));

					len = snprintf(buffer, sizeof(buffer)-1, "%.6f", iter->floatMax);
					buffer[sizeof(buffer)-1] = '\0';

					// trim trailing 0s after the decimal point
					pChar = buffer + len - 1;
					while (pChar >= buffer)
					{
						if (*pChar == '.')
						{
							*pChar = '\0';
							break;
						}
						else if (*pChar != '0')
						{
							++pChar;
							*pChar = '\0';
							break;
						}

						--pChar;
					}

					m_textboxMaxFloatValue->SetText(Unicode::narrowToWide(buffer));
				}
				else if ((iter->comparison == AuctionQueryHeadersMessage::SCC_string_equal) ||
					(iter->comparison == AuctionQueryHeadersMessage::SCC_string_not_equal) ||
					(iter->comparison == AuctionQueryHeadersMessage::SCC_string_contain) ||
					(iter->comparison == AuctionQueryHeadersMessage::SCC_string_not_contain))
				{
					if (m_comboboxStringCompare->IsVisible())
					{
						int comboIndex = -1;
						if (iter->comparison == AuctionQueryHeadersMessage::SCC_string_equal)
							comboIndex = getComboBoxIndexFromIndexName(*m_comboboxStringCompare, std::string("equal"));
						else if (iter->comparison == AuctionQueryHeadersMessage::SCC_string_not_equal)
							comboIndex = getComboBoxIndexFromIndexName(*m_comboboxStringCompare, std::string("notEqual"));
						else if (iter->comparison == AuctionQueryHeadersMessage::SCC_string_contain)
							comboIndex = getComboBoxIndexFromIndexName(*m_comboboxStringCompare, std::string("contain"));
						else if (iter->comparison == AuctionQueryHeadersMessage::SCC_string_not_contain)
							comboIndex = getComboBoxIndexFromIndexName(*m_comboboxStringCompare, std::string("notContain"));

						if (comboIndex >= 0)
							m_comboboxStringCompare->SetSelectedIndex(comboIndex);

						if (m_pageStringValue->IsVisible())
						{
							m_textboxStringValue->SetText(Unicode::narrowToWide(iter->stringValue));
						}
						else if (m_pageComboValue->IsVisible())
						{
							m_comboboxValue->SetProperty(UIComboBox::PropertyName::Editable, Unicode::narrowToWide ("true"));
							m_comboboxValue->SetProperty(UIComboBox::PropertyName::SelectedText, Unicode::narrowToWide(iter->stringValue));
						}
					}
					else if (m_comboboxEnumCompare->IsVisible())
					{
						int comboIndex = -1;
						if (iter->comparison == AuctionQueryHeadersMessage::SCC_string_equal)
							comboIndex = getComboBoxIndexFromIndexName(*m_comboboxEnumCompare, std::string("equal"));
						else if (iter->comparison == AuctionQueryHeadersMessage::SCC_string_not_equal)
							comboIndex = getComboBoxIndexFromIndexName(*m_comboboxEnumCompare, std::string("notEqual"));

						if (comboIndex >= 0)
							m_comboboxEnumCompare->SetSelectedIndex(comboIndex);

						if (m_pageComboValue->IsVisible())
						{
							comboIndex = getComboBoxIndexFromIndexName(*m_comboboxValue, iter->stringValue);
							if (comboIndex >= 0)
								m_comboboxValue->SetSelectedIndex(comboIndex);
						}
					}
				}

				break;
			}

			--selectedRowLogicalIndex;
		}

		enableButtonsForItemAttributeFilter();
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::onItemTypeListChanged (const AuctionManagerClient::Messages::ItemTypeListChanged::Payload &)
{
	updateFilterTree();
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::onResourceTypeListChanged (const AuctionManagerClient::Messages::ResourceTypeListChanged::Payload &)
{
	updateFilterTree();
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::reset ()
{
	const ClientObject * const obj = dynamic_cast<const ClientObject *>(AuctionManagerClient::getMarketObjectId ().getObject ());

	if (!obj)
		return;

	if (m_radioCity)
	{
		m_radioCity->SetChecked (true);
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::updateFilterTree()
{
	if (m_treeTypes && ((m_itemTypeListVersion != AuctionManagerClient::getItemTypeListVersion()) || (m_resourceTypeListVersion != AuctionManagerClient::getResourceTypeListVersion())))
	{
		m_currentlySelectedRowDSC = NULL;

		int const selectedObjectType = getSelectedObjectType();

		UIDataSourceContainer * const dsc = m_treeTypes->GetDataSourceContainer ();
		NOT_NULL (dsc);

		dsc->Attach (0);
		m_treeTypes->SetDataSourceContainer (0);
		dsc->Clear ();
		updateTypeTreeLabels (*dsc);
		m_treeTypes->SetDataSourceContainer (dsc);
		dsc->Detach (0);

		if (selectedObjectType != 0)
			setSelectedObjectType (selectedObjectType, false);
		else
			m_treeTypes->SelectRow (0);

		m_itemTypeListVersion = AuctionManagerClient::getItemTypeListVersion ();
		m_resourceTypeListVersion = AuctionManagerClient::getResourceTypeListVersion ();
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::enableButtonsForItemAttributeFilter()
{
	if (!m_enableItemAttributeFilter)
		return;

	int const selectedAttributeName = static_cast<int>(m_comboboxAttributeName->GetSelectedIndex());
	int const filterCount = m_attributeFilterTable->GetRowCount();
	int const selectedFilter = static_cast<int>(m_attributeFilterTable->GetLastSelectedRow());

	m_buttonAddFilterAttribute->SetEnabled(selectedAttributeName >= 0);
	m_buttonUpdateFilterAttribute->SetEnabled((selectedAttributeName >= 0) && (filterCount > 0) && (selectedFilter >= 0));
	m_buttonRemoveFilterAttribute->SetEnabled((filterCount > 0) && (selectedFilter >= 0));
	m_buttonRemoveAllFilterAttribute->SetEnabled(filterCount > 0);
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::enableControlsForItemAttributeFilter(bool refreshValueCombobox)
{
	if (!m_enableItemAttributeFilter)
		return;

	bool const enabled = (m_enableItemAttributeFilter->IsVisible() && m_enableItemAttributeFilter->IsChecked());

	m_comboboxMatchAllAny->SetVisible(enabled);
	m_attributeFilterTableHeader->SetVisible(enabled);
	m_attributeFilterTable->SetVisible(enabled);
	m_attributeFilterTableScrollbar->SetVisible(enabled);
	m_buttonAddFilterAttribute->SetVisible(enabled);
	m_buttonUpdateFilterAttribute->SetVisible(enabled);
	m_buttonRemoveFilterAttribute->SetVisible(enabled);
	m_buttonRemoveAllFilterAttribute->SetVisible(enabled);
	m_labelAttributeName->SetVisible(enabled);
	m_comboboxAttributeName->SetVisible(enabled);

	if (!enabled)
	{
		m_labelMinValue->SetVisible(false);
		m_pageMinIntValue->SetVisible(false);
		m_pageMinFloatValue->SetVisible(false);
		m_comboboxStringCompare->SetVisible(false);
		m_comboboxEnumCompare->SetVisible(false);
		m_labelMaxValue->SetVisible(false);
		m_labelValue->SetVisible(false);
		m_pageMaxIntValue->SetVisible(false);
		m_pageMaxFloatValue->SetVisible(false);
		m_pageStringValue->SetVisible(false);
		m_pageComboValue->SetVisible(false);
	}
	else
	{
		int selectedObjectTemplateId;
		bool selectedObjectIsResourceContainer;
		bool selectedObjectIsSpecialGeneralGot;
		int factoryCrateObjectType = 0;
		int const selectedObjectType = getSelectedObjectType(selectedObjectTemplateId, selectedObjectIsResourceContainer, selectedObjectIsSpecialGeneralGot);

		std::string selectedAttribute;
		m_comboboxAttributeName->GetSelectedIndexName(selectedAttribute);

		CommoditiesAdvancedSearchAttribute::SearchAttribute const * selectedSearchAttribute = CommoditiesAdvancedSearchAttribute::getSearchAttribute(selectedObjectType, selectedAttribute);
		if (!selectedSearchAttribute)
		{
			// for factory crates, the selected attribute name could be from the item type inside the crate
			if (selectedObjectType == SharedObjectTemplate::GOT_misc_factory_crate)
			{
				// if the factory crate node is selected, the value of the "object_type"
				// search condition (if specified) indicates the item type inside the crate
				if (selectedObjectTemplateId == 0)
				{
					std::map<std::pair<int, int>, std::list<AuctionQueryHeadersMessage::SearchCondition> >::const_iterator const iterFindFactoryCrateSearchAttribute = s_attributeFilter.find(std::make_pair(selectedObjectType, selectedObjectTemplateId));
					if (iterFindFactoryCrateSearchAttribute != s_attributeFilter.end())
					{
						for (std::list<AuctionQueryHeadersMessage::SearchCondition>::const_iterator iterSearchCondition = iterFindFactoryCrateSearchAttribute->second.begin(); iterSearchCondition != iterFindFactoryCrateSearchAttribute->second.end(); ++iterSearchCondition)
						{
							if ((iterSearchCondition->attributeNameCrc == s_objectTypeAttributeNameCrc) && (iterSearchCondition->comparison == AuctionQueryHeadersMessage::SCC_string_equal))
							{
								if (0 == iterSearchCondition->stringValue.find("@got_n:"))
								{
									factoryCrateObjectType = GameObjectTypes::getGameObjectType(iterSearchCondition->stringValue.substr(7));
									selectedSearchAttribute = CommoditiesAdvancedSearchAttribute::getSearchAttribute(factoryCrateObjectType, selectedAttribute);
								}
								else
								{
									// value may be aliased
									CommoditiesAdvancedSearchAttribute::SearchAttribute const * const factoryCrateObjectTypeSearchAttribute = CommoditiesAdvancedSearchAttribute::getSearchAttribute(selectedObjectType, "object_type");
									if (factoryCrateObjectTypeSearchAttribute)
									{
										for (std::map<std::string, std::string>::const_iterator iterAlias = factoryCrateObjectTypeSearchAttribute->enumValueAliasList.begin(); iterAlias != factoryCrateObjectTypeSearchAttribute->enumValueAliasList.end(); ++iterAlias)
										{
											if (iterAlias->second == iterSearchCondition->stringValue)
											{
												if (0 == iterAlias->first.find("@got_n:"))
												{
													factoryCrateObjectType = GameObjectTypes::getGameObjectType(iterAlias->first.substr(7));
													selectedSearchAttribute = CommoditiesAdvancedSearchAttribute::getSearchAttribute(factoryCrateObjectType, selectedAttribute);
												}

												break;
											}
										}
									}
								}

								break;
							}
						}
					}
				}
				// if an item node under the factory node is selected, get the item
				// type value from it which indicates the item type inside the crate
				else
				{
					const AuctionManagerClient::ItemTypeNameListServer & factoryCrateItemTypeNameList = AuctionManagerClient::getItemTypeNameListServer(selectedObjectType);
					AuctionManagerClient::ItemTypeNameListServer::const_iterator const iterFindTemplate = factoryCrateItemTypeNameList.find(selectedObjectTemplateId);
					if (iterFindTemplate != factoryCrateItemTypeNameList.end() && (iterFindTemplate->second.first != SharedObjectTemplate::GOT_misc_factory_crate))
					{
						factoryCrateObjectType = iterFindTemplate->second.first;
						selectedSearchAttribute = CommoditiesAdvancedSearchAttribute::getSearchAttribute(factoryCrateObjectType, selectedAttribute);
					}
				}
			}
		}

		if (!selectedSearchAttribute || (selectedSearchAttribute->attributeDataType == CommoditiesAdvancedSearchAttribute::SADT_int))
		{
			m_labelMinValue->SetVisible(true);
			m_pageMinIntValue->SetVisible(true);
			m_pageMinFloatValue->SetVisible(false);
			m_comboboxStringCompare->SetVisible(false);
			m_comboboxEnumCompare->SetVisible(false);
			m_labelMaxValue->SetVisible(true);
			m_labelValue->SetVisible(false);
			m_pageMaxIntValue->SetVisible(true);
			m_pageMaxFloatValue->SetVisible(false);
			m_pageStringValue->SetVisible(false);
			m_pageComboValue->SetVisible(false);
		}
		else if (selectedSearchAttribute->attributeDataType == CommoditiesAdvancedSearchAttribute::SADT_float)
		{
			m_labelMinValue->SetVisible(true);
			m_pageMinIntValue->SetVisible(false);
			m_pageMinFloatValue->SetVisible(true);
			m_comboboxStringCompare->SetVisible(false);
			m_comboboxEnumCompare->SetVisible(false);
			m_labelMaxValue->SetVisible(true);
			m_labelValue->SetVisible(false);
			m_pageMaxIntValue->SetVisible(false);
			m_pageMaxFloatValue->SetVisible(true);
			m_pageStringValue->SetVisible(false);
			m_pageComboValue->SetVisible(false);
		}
		else if (selectedSearchAttribute->attributeDataType == CommoditiesAdvancedSearchAttribute::SADT_string)
		{
			m_labelMinValue->SetVisible(false);
			m_pageMinIntValue->SetVisible(false);
			m_pageMinFloatValue->SetVisible(false);
			m_comboboxStringCompare->SetVisible(true);
			m_comboboxEnumCompare->SetVisible(false);
			m_labelMaxValue->SetVisible(false);
			m_labelValue->SetVisible(true);
			m_pageMaxIntValue->SetVisible(false);
			m_pageMaxFloatValue->SetVisible(false);

			if (selectedSearchAttribute->defaultSearchValueList.empty())
			{
				m_pageStringValue->SetVisible(true);
				m_pageComboValue->SetVisible(false);
			}
			else
			{
				m_pageStringValue->SetVisible(false);
				m_pageComboValue->SetVisible(true);
				m_comboboxValue->SetProperty(UIComboBox::PropertyName::Editable, Unicode::narrowToWide ("true"));

				if (refreshValueCombobox)
				{
					Unicode::String selectedValue;
					m_comboboxValue->GetProperty(UIComboBox::PropertyName::SelectedText, selectedValue);
					m_comboboxValue->SetProperty(UIComboBox::PropertyName::SelectedText, Unicode::emptyString);
					std::string const narrowSelectedValue(Unicode::wideToNarrow(selectedValue));

					m_comboboxValue->SetSelectedIndex(-1);
					m_comboboxValue->Clear();

					int defaultIndex = -1;
					int index = 0;
					for (std::set<std::string>::const_iterator iter = selectedSearchAttribute->defaultSearchValueList.begin(); iter != selectedSearchAttribute->defaultSearchValueList.end(); ++iter)
					{
						m_comboboxValue->AddItem(Unicode::narrowToWide(*iter), *iter);

						if (narrowSelectedValue == *iter)
						{
							defaultIndex = index;
						}

						++index;
					}

					if (defaultIndex >= 0)
						m_comboboxValue->SetSelectedIndex(defaultIndex);
				}
			}
		}
		else if (selectedSearchAttribute->attributeDataType == CommoditiesAdvancedSearchAttribute::SADT_enum)
		{
			m_labelMinValue->SetVisible(false);
			m_pageMinIntValue->SetVisible(false);
			m_pageMinFloatValue->SetVisible(false);
			m_comboboxStringCompare->SetVisible(false);
			m_comboboxEnumCompare->SetVisible(true);
			m_labelMaxValue->SetVisible(false);
			m_labelValue->SetVisible(true);
			m_pageMaxIntValue->SetVisible(false);
			m_pageMaxFloatValue->SetVisible(false);
			m_pageStringValue->SetVisible(false);
			m_pageComboValue->SetVisible(true);
			m_comboboxValue->SetProperty(UIComboBox::PropertyName::Editable, Unicode::narrowToWide ("false"));

			if (refreshValueCombobox)
			{
				// temporarily set editable to true in order to retrieve the text value
				m_comboboxValue->SetProperty(UIComboBox::PropertyName::Editable, Unicode::narrowToWide ("true"));
				Unicode::String selectedValue;
				m_comboboxValue->GetProperty(UIComboBox::PropertyName::SelectedText, selectedValue);
				m_comboboxValue->SetProperty(UIComboBox::PropertyName::SelectedText, Unicode::emptyString);

				m_comboboxValue->SetSelectedIndex(-1);
				m_comboboxValue->Clear();

				m_comboboxValue->SetProperty(UIComboBox::PropertyName::Editable, Unicode::narrowToWide ("false"));

				std::map<Unicode::String, std::pair<Unicode::String, std::string> > const & enumDisplayString = getAttributeFilterEnumDisplayString(((factoryCrateObjectType > 0) ? factoryCrateObjectType : selectedObjectType), selectedAttribute);
				if (!enumDisplayString.empty())
				{
					int defaultIndex = 0;
					int index = 0;
					for (std::map<Unicode::String, std::pair<Unicode::String, std::string> >::const_iterator iter = enumDisplayString.begin(); iter != enumDisplayString.end(); ++iter)
					{
						m_comboboxValue->AddItem(iter->second.first, iter->second.second);

						if (selectedValue == iter->second.first)
						{
							defaultIndex = index;
						}

						++index;
					}

					m_comboboxValue->SetSelectedIndex(defaultIndex);
				}
			}
		}
		else
		{
			m_labelMinValue->SetVisible(true);
			m_pageMinIntValue->SetVisible(true);
			m_pageMinFloatValue->SetVisible(false);
			m_comboboxStringCompare->SetVisible(false);
			m_comboboxEnumCompare->SetVisible(false);
			m_labelMaxValue->SetVisible(true);
			m_labelValue->SetVisible(false);
			m_pageMaxIntValue->SetVisible(true);
			m_pageMaxFloatValue->SetVisible(false);
			m_pageStringValue->SetVisible(false);
			m_pageComboValue->SetVisible(false);
		}
	}

	m_requiredAttribute->SetVisible(enabled);
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::refreshItemAttributeFilter()
{
	if (!m_enableItemAttributeFilter)
		return;

	int selectedObjectTemplateId;
	bool selectedObjectIsResourceContainer;
	bool selectedObjectIsSpecialGeneralGot;
	int const selectedObjectType = getSelectedObjectType(selectedObjectTemplateId, selectedObjectIsResourceContainer, selectedObjectIsSpecialGeneralGot);

	// refresh the state of the enable attribute search checkbox
	m_enableItemAttributeFilter->SetChecked((s_enableItemAttributeFilter.count(std::make_pair(selectedObjectType, selectedObjectTemplateId)) >= 1));

	// refresh the match all/any state
	m_comboboxMatchAllAny->SetSelectedIndex(static_cast<long>(getItemAttributeFilterMatchAllAny(selectedObjectType, selectedObjectTemplateId)));

	// refresh the search attribute filters
	{
		UITableModelDefault * const tableModel = safe_cast<UITableModelDefault *>(m_attributeFilterTable->GetTableModel());
		if (tableModel)
		{
			tableModel->ClearTable();

			long const selectedRow = m_attributeFilterTable->GetLastSelectedRow();
			if (selectedRow >= 0)
				m_attributeFilterTable->RemoveRowSelection(selectedRow);
		}

		std::map<std::pair<int, int>, std::list<AuctionQueryHeadersMessage::SearchCondition> >::const_iterator const iterFind1 = s_attributeFilter.find(std::make_pair(selectedObjectType, selectedObjectTemplateId));
		if (iterFind1 != s_attributeFilter.end())
		{
			std::map<std::pair<int, int>, std::list<std::pair<Unicode::String, Unicode::String> > >::const_iterator const iterFind2 = s_attributeFilterDisplayData.find(std::make_pair(selectedObjectType, selectedObjectTemplateId));
			if (iterFind2 != s_attributeFilterDisplayData.end())
			{
				std::list<AuctionQueryHeadersMessage::SearchCondition>::const_iterator iter1 = iterFind1->second.begin();
				std::list<std::pair<Unicode::String, Unicode::String> >::const_iterator iter2 = iterFind2->second.begin();
				for (; ((iter1 != iterFind1->second.end()) && (iter2 != iterFind2->second.end())); ++iter1, ++iter2)
				{
					addAttributeFilter(iter2->first, iter2->second, iter1->requiredAttribute, *m_attributeFilterTable, false);
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::populateSearchableAttributeNameCombo(std::map<Unicode::String, std::pair<Unicode::String, std::string> > const & searchAttribute, std::map<Unicode::String, std::pair<Unicode::String, std::string> > const * additionalSearchAttribute, std::string const & excludeSearchAttribute)
{
	if (!m_enableItemAttributeFilter)
		return;

	std::string selectedAttribute;
	if (m_comboboxAttributeName->GetSelectedIndex() >= 0)
	{
		m_comboboxAttributeName->GetSelectedIndexName(selectedAttribute);
	}

	m_comboboxAttributeName->SetSelectedIndex(-1);
	m_comboboxAttributeName->Clear();

	long selectedIndex = -1;
	long count = 0;
	for (std::map<Unicode::String, std::pair<Unicode::String, std::string> >::const_iterator iter = searchAttribute.begin(); iter != searchAttribute.end(); ++iter)
	{
		if (excludeSearchAttribute == iter->second.second)
		{
			continue;
		}

		if ((iter->second.second == selectedAttribute) && (selectedIndex < 0))
		{
			selectedIndex = count;
		}

		m_comboboxAttributeName->AddItem(iter->second.first, iter->second.second);
		++count;
	}

	if (additionalSearchAttribute && !additionalSearchAttribute->empty())
	{
		for (std::map<Unicode::String, std::pair<Unicode::String, std::string> >::const_iterator iter = additionalSearchAttribute->begin(); iter != additionalSearchAttribute->end(); ++iter)
		{
			if (searchAttribute.count(iter->first) <= 0)
			{
				if (excludeSearchAttribute == iter->second.second)
				{
					continue;
				}

				if ((iter->second.second == selectedAttribute) && (selectedIndex < 0))
				{
					selectedIndex = count;
				}

				m_comboboxAttributeName->AddItem(iter->second.first, iter->second.second);
				++count;
			}
		}
	}

	if (selectedIndex >= 0)
	{
		m_comboboxAttributeName->SetSelectedIndex(selectedIndex);
	}
	else
	{
		m_textboxMinIntValue->SetText(Unicode::emptyString);
		m_textboxMaxIntValue->SetText(Unicode::emptyString);
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::saveNamePriceFilter()
{
	if (m_textboxTextFilter && m_treeTypes)
	{
		int selectedObjectTemplateId;
		bool selectedObjectIsResourceContainer;
		bool selectedObjectIsSpecialGeneralGot;
		int const selectedObjectType = getSelectedObjectType(selectedObjectTemplateId, selectedObjectIsResourceContainer, selectedObjectIsSpecialGeneralGot);

		s_nameFilter[std::make_pair(selectedObjectType, selectedObjectTemplateId)] = m_textboxTextFilter->GetLocalText();
	}

	if (m_textboxMinPriceFilter)
	{
		s_minPriceFilter = m_textboxMinPriceFilter->GetLocalText();
	}

	if (m_textboxMaxPriceFilter)
	{
		s_maxPriceFilter = m_textboxMaxPriceFilter->GetLocalText();
	}

	if (m_includeEntranceFee)
	{
		s_includeEntranceFee = m_includeEntranceFee->IsChecked();
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::restoreNamePriceFilter()
{
	if (m_textboxTextFilter && m_treeTypes)
	{
		int selectedObjectTemplateId;
		bool selectedObjectIsResourceContainer;
		bool selectedObjectIsSpecialGeneralGot;
		int const selectedObjectType = getSelectedObjectType(selectedObjectTemplateId, selectedObjectIsResourceContainer, selectedObjectIsSpecialGeneralGot);

		std::map<std::pair<int, int>, Unicode::String>::const_iterator const iterFind = s_nameFilter.find(std::make_pair(selectedObjectType, selectedObjectTemplateId));
		if (iterFind != s_nameFilter.end())
		{
			m_textboxTextFilter->SetText(iterFind->second);
		}
		else
		{
			m_textboxTextFilter->SetText(Unicode::emptyString);
		}
	}

	if (m_textboxMinPriceFilter)
	{
		m_textboxMinPriceFilter->SetText(s_minPriceFilter);
	}

	if (m_textboxMaxPriceFilter)
	{
		m_textboxMaxPriceFilter->SetText(s_maxPriceFilter);
	}

	if (m_includeEntranceFee)
	{
		m_includeEntranceFee->SetChecked(s_includeEntranceFee);
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::requestUpdate ()
{
	setIsUpdating (true);
	m_requestUpdateNextFrame = true;
}

//----------------------------------------------------------------------

int SwgCuiAuctionFilter::getSelectedLocation () const 
{
	if (m_radioGalaxy && m_radioGalaxy->IsChecked ())
		return 0;
	if (m_radioPlanet && m_radioPlanet->IsChecked ())
		return 1;
	if (m_radioCity && m_radioCity->IsChecked ())
		return 2;
	
	return 3;
	
}

//----------------------------------------------------------------------

const Unicode::String & SwgCuiAuctionFilter::getTextFilterAll () const 
{
	if (m_textboxTextFilter)
		return m_textboxTextFilter->GetLocalText();

	return Unicode::emptyString;
}

//----------------------------------------------------------------------

const Unicode::String & SwgCuiAuctionFilter::getTextFilterAny () const 
{
	return Unicode::emptyString;
}

//----------------------------------------------------------------------

int SwgCuiAuctionFilter::getPriceFilterMin () const 
{
	if (m_textboxMinPriceFilter != 0)
	{
		Unicode::String const & widePrice = m_textboxMinPriceFilter->GetLocalText();
		std::string narrowPrice = Unicode::wideToNarrow(widePrice);

		if (!narrowPrice.empty())
		{
			int const price = strtol(narrowPrice.c_str(), 0, 10);
			return price;
		}
	}

	return 0;
}

//----------------------------------------------------------------------

int SwgCuiAuctionFilter::getPriceFilterMax () const 
{
	if (m_textboxMaxPriceFilter != 0)
	{
		Unicode::String const & widePrice = m_textboxMaxPriceFilter->GetLocalText();
		std::string narrowPrice = Unicode::wideToNarrow(widePrice);

		if (!narrowPrice.empty())
		{
			int const price = strtol(narrowPrice.c_str(), 0, 10);
			return price;
		}
	}

	return 0;
}

//----------------------------------------------------------------------

bool SwgCuiAuctionFilter::getPriceFilterIncludesEntranceFee () const 
{
	if (m_includeEntranceFee != 0)
	{
		return m_includeEntranceFee->IsChecked();
	}

	return false;
}

//----------------------------------------------------------------------

std::list<AuctionQueryHeadersMessage::SearchCondition> const & SwgCuiAuctionFilter::getItemAttributeFilter(AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny & matchAllAny) const
{
	static std::list<AuctionQueryHeadersMessage::SearchCondition> const empty;

	if (!m_enableItemAttributeFilter)
		return empty;

	int selectedObjectTemplateId;
	bool selectedObjectIsResourceContainer;
	bool selectedObjectIsSpecialGeneralGot;
	int const selectedObjectType = getSelectedObjectType(selectedObjectTemplateId, selectedObjectIsResourceContainer, selectedObjectIsSpecialGeneralGot);

	if (s_enableItemAttributeFilter.count(std::make_pair(selectedObjectType, selectedObjectTemplateId)) >= 1)
	{
		std::map<std::pair<int, int>, std::list<AuctionQueryHeadersMessage::SearchCondition> >::const_iterator const iterFind = s_attributeFilter.find(std::make_pair(selectedObjectType, selectedObjectTemplateId));
		if (iterFind != s_attributeFilter.end())
		{
			matchAllAny = getItemAttributeFilterMatchAllAny(selectedObjectType, selectedObjectTemplateId);
			return iterFind->second;
		}
	}

	return empty;
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::checkpointTextPriceFilterValues()
{
	m_checkpointTextFilterAll = getTextFilterAll();
	m_checkpointTextFilterAny = getTextFilterAny();
	m_checkpointPriceFilterMin = getPriceFilterMin();
	m_checkpointPriceFilterMax = getPriceFilterMax();
	m_checkpointPriceFilterIncludesEntranceFee = getPriceFilterIncludesEntranceFee();

	if (!m_enableItemAttributeFilter)
	{
		m_checkpointAttributeFilter.clear();
		return;
	}

	int selectedObjectTemplateId;
	bool selectedObjectIsResourceContainer;
	bool selectedObjectIsSpecialGeneralGot;
	int const selectedObjectType = getSelectedObjectType(selectedObjectTemplateId, selectedObjectIsResourceContainer, selectedObjectIsSpecialGeneralGot);

	m_checkpointItemAttributeFilterMatchAllAny = getItemAttributeFilterMatchAllAny(selectedObjectType, selectedObjectTemplateId);

	if (s_enableItemAttributeFilter.count(std::make_pair(selectedObjectType, selectedObjectTemplateId)) >= 1)
	{
		std::map<std::pair<int, int>, std::list<AuctionQueryHeadersMessage::SearchCondition> >::const_iterator const iterFind = s_attributeFilter.find(std::make_pair(selectedObjectType, selectedObjectTemplateId));
		if (iterFind != s_attributeFilter.end())
		{
			m_checkpointAttributeFilter = iterFind->second;
		}
		else
		{
			m_checkpointAttributeFilter.clear();
		}
	}
	else
	{
		m_checkpointAttributeFilter.clear();
	}
}

//----------------------------------------------------------------------

bool SwgCuiAuctionFilter::hasTextPriceFilterValuesChangedSinceCheckpoint() const
{
	if ((m_checkpointTextFilterAll != getTextFilterAll()) ||
		(m_checkpointTextFilterAny != getTextFilterAny()) ||
		(m_checkpointPriceFilterMin != getPriceFilterMin()) ||
		(m_checkpointPriceFilterMax != getPriceFilterMax()) ||
		(m_checkpointPriceFilterIncludesEntranceFee != getPriceFilterIncludesEntranceFee()))
	{
		return true;
	}

	if (!m_enableItemAttributeFilter)
		return false;

	int selectedObjectTemplateId;
	bool selectedObjectIsResourceContainer;
	bool selectedObjectIsSpecialGeneralGot;
	int const selectedObjectType = getSelectedObjectType(selectedObjectTemplateId, selectedObjectIsResourceContainer, selectedObjectIsSpecialGeneralGot);

	if (s_enableItemAttributeFilter.count(std::make_pair(selectedObjectType, selectedObjectTemplateId)) >= 1)
	{
		std::map<std::pair<int, int>, std::list<AuctionQueryHeadersMessage::SearchCondition> >::const_iterator const iterFind = s_attributeFilter.find(std::make_pair(selectedObjectType, selectedObjectTemplateId));
		if (iterFind != s_attributeFilter.end())
		{
			if (m_checkpointAttributeFilter != iterFind->second)
				return true;
			else if (!m_checkpointAttributeFilter.empty() && (m_checkpointItemAttributeFilterMatchAllAny != getItemAttributeFilterMatchAllAny(selectedObjectType, selectedObjectTemplateId)))
				return true;
		}
		else
		{
			if (!m_checkpointAttributeFilter.empty())
				return true;
		}
	}
	else
	{
		if (!m_checkpointAttributeFilter.empty())
			return true;
	}

	return false;
}

//----------------------------------------------------------------------

int SwgCuiAuctionFilter::getSelectedObjectType (int & selectedObjectTemplateId, bool & selectedObjectIsResourceContainer, bool & selectedObjectIsSpecialGeneralGot) const
{
	int got = SharedObjectTemplate::GOT_none;
	selectedObjectTemplateId = 0;
	selectedObjectIsResourceContainer = false;
	selectedObjectIsSpecialGeneralGot = false;

	if (m_treeTypes)
	{
		const long selectedRow = m_treeTypes->GetLastSelectedRow ();
		const UIDataSourceContainer * const dsc = m_treeTypes->GetDataSourceContainerAtRow  (selectedRow);
		
		if (dsc)
		{
			if (!dsc->GetPropertyInteger (DataProperties::TypeTree::ItemGot, got))
				got = SharedObjectTemplate::GOT_none;

			if (!dsc->GetPropertyInteger (DataProperties::TypeTree::ItemTemplateCrc, selectedObjectTemplateId))
				selectedObjectTemplateId = 0;

			int itemIsResourceContainer;
			if ((dsc->GetPropertyInteger (DataProperties::TypeTree::ItemIsResourceContainer, itemIsResourceContainer)) && (itemIsResourceContainer == 1))
				selectedObjectIsResourceContainer = true;
			else
				selectedObjectIsResourceContainer = false;	

			int itemIsSpecialGeneralGot;
			if ((dsc->GetPropertyInteger (DataProperties::TypeTree::ItemIsSpecialGeneralGot, itemIsSpecialGeneralGot)) && (itemIsSpecialGeneralGot == 1))
				selectedObjectIsSpecialGeneralGot = true;
			else
				selectedObjectIsSpecialGeneralGot = false;		
		}
	}

	return got;
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::update (float deltaTimeSecs)
{
	CuiMediator::update (deltaTimeSecs);

	setIsUpdating (false);
	if (m_requestUpdateNextFrame)
	{
		m_transceiverChanged->emitMessage (*this);
	}
}

//----------------------------------------------------------------------

int SwgCuiAuctionFilter::getSelectedObjectType         () const
{
	int type = 0;

	if (m_treeTypes)
	{
		const long selectedRow = m_treeTypes->GetLastSelectedRow ();
		const UIDataSourceContainer * const dsc = m_treeTypes->GetDataSourceContainerAtRow  (selectedRow);
		if (dsc)
		{
			if (!dsc->GetPropertyInteger (DataProperties::TypeTree::ItemGot, type))
				type = 0;
		}
	}

	return type;
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::setSelectedObjectType         (int type, bool requestRefresh)
{
	if (m_treeTypes)
	{
		int got = 0;
		m_ignoreFilterChangeRequestUpdate = !requestRefresh;

		typedef stdvector<const UIDataSourceContainer *>::fwd DscVector;
		static DscVector dscVector;

		dscVector.clear ();

		const UIDataSourceContainer * top = m_treeTypes->GetDataSourceContainer ();

		if (top)		
		{
			dscVector.push_back (top);

			while (!dscVector.empty ())
			{
				const UIDataSourceContainer * const dsc = dscVector.back ();
				dscVector.pop_back ();

				if (dsc->GetPropertyInteger (DataProperties::TypeTree::ItemGot, got) && (got == type))
				{
					int row = 0;
					if (m_treeTypes->FindDataNodeByDataSource (*dsc, row))
					{
						m_treeTypes->SelectRow   (row);
						m_treeTypes->ScrollToRow (row);
					}

					m_ignoreFilterChangeRequestUpdate = false;
					return;
				}

				const UIDataSourceContainer::DataSourceBaseList & dsbl = dsc->GetDataSourceBaseList ();
				for (UIDataSourceContainer::DataSourceBaseList::const_iterator it = dsbl.begin (); it != dsbl.end (); ++it)
				{
					const UIDataSourceBase * const dsb = *it;
					if (dsb && dsb->IsA (TUIDataSourceContainer))
					{
						dscVector.push_back (safe_cast<const UIDataSourceContainer *>(dsb));
					}
				}
			}
		}

		m_ignoreFilterChangeRequestUpdate = false;

		WARNING (true, ("SwgCuiAuctionFilter::setSelectedObjectType(%d) failed", type));
	}
}

//----------------------------------------------------------------------

void SwgCuiAuctionFilter::buildAttributeFilterDisplayString()
{
	s_attributeFilterDisplayString.clear();

	ObjectAttributeManager::AttributeVector attributeVector;
	Unicode::String attributeDisplayNameCategory;
	Unicode::String attributeDisplayNameKey;
	Unicode::String attributeDisplayName;
	Unicode::String attributeDetails;
	std::map<Unicode::String, std::pair<Unicode::String, std::string> >::const_iterator iterFindDupe;
	size_t crLfPos;

	GameObjectTypes::TypeStringMap const & got = GameObjectTypes::getTypeStringMap();
	for (GameObjectTypes::TypeStringMap::const_iterator iterGot = got.begin(); iterGot != got.end(); ++iterGot)
	{
		std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *> const & sa = CommoditiesAdvancedSearchAttribute::getSearchAttributeForGameObjectType(iterGot->first);
		if (sa.empty())
			continue;

		std::map<Unicode::String, std::pair<Unicode::String, std::string> > & attributeFilterDisplayString = s_attributeFilterDisplayString[iterGot->first];

		for (std::map<std::string, CommoditiesAdvancedSearchAttribute::SearchAttribute const *>::const_iterator iterAttribute = sa.begin(); iterAttribute != sa.end(); ++iterAttribute)
		{
			attributeVector.clear();
			attributeVector.push_back(std::make_pair(iterAttribute->second->attributeName, Unicode::emptyString));

			attributeDisplayNameCategory.clear();
			attributeDisplayNameKey.clear();
			attributeDetails.clear();
			ObjectAttributeManager::formatAttributes(attributeVector, attributeDetails, &attributeDisplayNameCategory, &attributeDisplayNameKey, false);

			if (!attributeDisplayNameKey.empty())
			{
				crLfPos = attributeDisplayNameKey.find_first_of(Unicode::narrowToWide("\r\n"));
				if (crLfPos != std::string::npos)
				{
					attributeDisplayNameKey = attributeDisplayNameKey.substr(0, crLfPos);
				}
			}

			if (!attributeDisplayNameCategory.empty())
			{
				crLfPos = attributeDisplayNameCategory.find_first_of(Unicode::narrowToWide("\r\n"));
				if (crLfPos != std::string::npos)
				{
					attributeDisplayNameCategory = attributeDisplayNameCategory.substr(0, crLfPos);
				}
			}

			FATAL(attributeDisplayNameKey.empty(), ("Empty display string for search attribute (%s) for game object type (%s)", iterAttribute->second->attributeName.c_str(), iterGot->second.c_str()));

			attributeDisplayName = attributeDisplayNameKey;
			if (!attributeDisplayNameCategory.empty())
			{
				attributeDisplayName += Unicode::narrowToWide(" (");
				attributeDisplayName += attributeDisplayNameCategory;
				attributeDisplayName += Unicode::narrowToWide(")");
			}

			attributeDisplayName = Unicode::getTrim(attributeDisplayName);
			iterFindDupe = attributeFilterDisplayString.find(Unicode::toLower(attributeDisplayName));
			FATAL((iterFindDupe != attributeFilterDisplayString.end()), ("Search attribute (%s) for game object type (%s) has display string (%s) that is the same as another search attribute (%s)", iterAttribute->second->attributeName.c_str(), iterGot->second.c_str(), Unicode::wideToNarrow(attributeDisplayName).c_str(), iterFindDupe->second.second.c_str()));

			attributeFilterDisplayString[Unicode::toLower(attributeDisplayName)] = std::make_pair(attributeDisplayName, iterAttribute->second->attributeName);

			// for enum type attribute, build the list of display string
			if ((iterAttribute->second->attributeDataType == CommoditiesAdvancedSearchAttribute::SADT_enum) && !iterAttribute->second->defaultSearchValueList.empty())
			{
				std::map<Unicode::String, std::pair<Unicode::String, std::string> > & enumDisplayString = s_attributeFilterEnumDisplayString[std::make_pair(iterGot->first, iterAttribute->second->attributeName)];

				for (std::set<std::string>::const_iterator iterEnumValue = iterAttribute->second->defaultSearchValueList.begin(); iterEnumValue != iterAttribute->second->defaultSearchValueList.end(); ++iterEnumValue)
				{
					attributeDisplayName = StringId::decodeString(Unicode::narrowToWide(*iterEnumValue));
					if (attributeDisplayName.empty())
						attributeDisplayName = Unicode::narrowToWide(*iterEnumValue);

					iterFindDupe = enumDisplayString.find(Unicode::toLower(attributeDisplayName));
					FATAL((iterFindDupe != enumDisplayString.end()), ("Enum value (%s) for search attribute (%s) for game object type (%s) has display string (%s) that is the same as another enum value (%s)", iterEnumValue->c_str(), iterAttribute->second->attributeName.c_str(), iterGot->second.c_str(), Unicode::wideToNarrow(attributeDisplayName).c_str(), iterFindDupe->second.second.c_str()));

					enumDisplayString[Unicode::toLower(attributeDisplayName)] = std::make_pair(attributeDisplayName, *iterEnumValue);
				}
			}
		}

#ifdef _DEBUG
		std::map<std::string, std::string> const & saAlias = CommoditiesAdvancedSearchAttribute::getSearchAttributeNameAliasesForGameObjectType(iterGot->first);
		if (!saAlias.empty())
		{
			for (std::map<std::string, std::string>::const_iterator iterAlias = saAlias.begin(); iterAlias != saAlias.end(); ++iterAlias)
			{
				// get attribute display name for the alias attribute name
				attributeVector.clear();
				attributeVector.push_back(std::make_pair(iterAlias->first, Unicode::emptyString));

				attributeDisplayNameCategory.clear();
				attributeDisplayNameKey.clear();
				attributeDetails.clear();
				ObjectAttributeManager::formatAttributes(attributeVector, attributeDetails, &attributeDisplayNameCategory, &attributeDisplayNameKey, false);

				if (!attributeDisplayNameKey.empty())
				{
					crLfPos = attributeDisplayNameKey.find_first_of(Unicode::narrowToWide("\r\n"));
					if (crLfPos != std::string::npos)
					{
						attributeDisplayNameKey = attributeDisplayNameKey.substr(0, crLfPos);
					}
				}

				if (!attributeDisplayNameCategory.empty())
				{
					crLfPos = attributeDisplayNameCategory.find_first_of(Unicode::narrowToWide("\r\n"));
					if (crLfPos != std::string::npos)
					{
						attributeDisplayNameCategory = attributeDisplayNameCategory.substr(0, crLfPos);
					}
				}

				attributeDisplayName = attributeDisplayNameKey;
				if (!attributeDisplayNameCategory.empty())
				{
					attributeDisplayName += Unicode::narrowToWide(" (");
					attributeDisplayName += attributeDisplayNameCategory;
					attributeDisplayName += Unicode::narrowToWide(")");
				}

				attributeDisplayName = Unicode::getTrim(attributeDisplayName);
				std::string const alias(Unicode::wideToNarrow(attributeDisplayName));

				// get attribute display name for the alias master attribute name
				attributeVector.clear();
				attributeVector.push_back(std::make_pair(iterAlias->second, Unicode::emptyString));

				attributeDisplayNameCategory.clear();
				attributeDisplayNameKey.clear();
				attributeDetails.clear();
				ObjectAttributeManager::formatAttributes(attributeVector, attributeDetails, &attributeDisplayNameCategory, &attributeDisplayNameKey, false);

				if (!attributeDisplayNameKey.empty())
				{
					crLfPos = attributeDisplayNameKey.find_first_of(Unicode::narrowToWide("\r\n"));
					if (crLfPos != std::string::npos)
					{
						attributeDisplayNameKey = attributeDisplayNameKey.substr(0, crLfPos);
					}
				}

				if (!attributeDisplayNameCategory.empty())
				{
					crLfPos = attributeDisplayNameCategory.find_first_of(Unicode::narrowToWide("\r\n"));
					if (crLfPos != std::string::npos)
					{
						attributeDisplayNameCategory = attributeDisplayNameCategory.substr(0, crLfPos);
					}
				}

				attributeDisplayName = attributeDisplayNameKey;
				if (!attributeDisplayNameCategory.empty())
				{
					attributeDisplayName += Unicode::narrowToWide(" (");
					attributeDisplayName += attributeDisplayNameCategory;
					attributeDisplayName += Unicode::narrowToWide(")");
				}

				attributeDisplayName = Unicode::getTrim(attributeDisplayName);
				std::string const aliasMaster(Unicode::wideToNarrow(attributeDisplayName));

				DEBUG_WARNING((alias != aliasMaster), ("commodities search attribute: for game object type (%d, %s), attribute name alias (%s) (%s) has a different display string than the alias master (%s) (%s), so may not need to be aliased", iterGot->first, iterGot->second.c_str(), iterAlias->first.c_str(), alias.c_str(), iterAlias->second.c_str(), aliasMaster.c_str()));
			}
		}
#endif
	}
}

//======================================================================
