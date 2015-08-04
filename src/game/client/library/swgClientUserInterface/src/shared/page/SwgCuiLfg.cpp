// ======================================================================
//
// SwgCuiLfg.cpp
// copyright (c) 2008 Sony Online Entertainment LLC
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiLfg.h"

#include <fstream>
#include <list>
#include <map>

#include "clientGame/Game.h"
#include "clientGame/GameNetwork.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiStringVariablesData.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedFoundation/BitArray.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/Os.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

#include "UIButton.h"
#include "UICheckbox.h"
#include "UIComboBox.h"
#include "UIComposite.h"
#include "UIData.h"
#include "UIPage.h"
#include "UIText.h"
#include "UITextbox.h"
#include "UIWidget.h"

namespace SwgCuiLfgNamespace
{

	UIString const stylePlus = Unicode::narrowToWide ("/Styles.New.buttons.controlPage.zoomO.style");
	UIString const styleMinus = Unicode::narrowToWide ("/Styles.New.buttons.controlPage.zoomI.style");

	UINarrowString const textfieldMax = "max";
	UINarrowString const textfieldMin = "min";
	UINarrowString const buttonTree = "buttonTree";
	UINarrowString const buttonNone = "buttonNone";
	UINarrowString const buttonAll = "buttonAll"; 
	UINarrowString const checkChosen = "checkChosen";
	UINarrowString const checkAll = "checkAll";
	UINarrowString const checkAny	= "checkAny";
	UINarrowString const buttonSave = "buttonSave";
	UINarrowString const buttonLoad = "buttonLoad";
				   
	UINarrowString const pageBranch	= "Sampletier1";
	UINarrowString const pageLeaf	= "Samplechild1";
	UINarrowString const pageMinMax = "SampleMinMax";

	UIColor const colorDarkYellow (128, 128, 0);
	UIColor const colorManilla (200, 200, 128);

	std::string const defaultPath = "profiles/Lfg";

	int const childOffset = 20;
	int const containerPadding = 10;
	
	float const disabledShade = 0.65f;

	typedef std::vector<LfgDataTable::LfgNode const *> NodeVector;
	typedef LfgDataTable::DefaultMatchConditionType MatchCondition;

	std::map< UIWidget *, UIPage *> s_checkLinks;
	std::map< UIWidget *, UIComposite *> s_buttonLinks;
	std::map< UIWidget *, UIWidget *> s_checkAnyAll;

	void getPlayerFilePath (std::string & path);	
	void errorCheckMinMax (LfgDataTable::LfgNode const * node, int & min, int & max);
}

using namespace SwgCuiLfgNamespace;

//----------------------------------------------------------------------

SwgCuiLfg::SwgCuiLfg(UIPage & page, bool externalCriteriaOnly )	:
CuiMediator("SwgCuiLfg", page),
UIEventCallback(),
m_samplePage (NULL),
m_criteriaPage (NULL),
m_sampleChild (NULL),
m_sampleMinMax (NULL),
m_stringLinks (NULL),
m_nodeLinks (NULL),
m_comboBox (NULL),
m_externalCriteriaOnly (externalCriteriaOnly)
{

	getCodeDataObject (TUIComposite,    m_samplePage,    "samplepage");
	getCodeDataObject (TUIComposite,    m_criteriaPage,  "criteriapage");
	getCodeDataObject (TUIComposite,    m_sampleChild,   "samplechild");
	getCodeDataObject (TUIComposite,    m_sampleMinMax,  "sampleminmax");
	getCodeDataObject (TUIComboBox,     m_comboBox,      "comboSaveLoad", true);

	if (m_comboBox)
		m_comboBox->AddCallback(this);

	UIButton * tempButton  = static_cast<UIButton *> (getPage().GetChild(buttonSave.c_str()));
	
	if (tempButton)
		tempButton->AddCallback(this);

	tempButton  = static_cast<UIButton *> (getPage().GetChild(buttonLoad.c_str()));

	if (tempButton)
		tempButton->AddCallback(this);

	m_samplePage->SetVisible(false);
	m_sampleChild->SetVisible(false);
	m_sampleMinMax->SetVisible(false);

	m_stringLinks = new StringMap;
	m_nodeLinks   = new PageMap;

	loadTable();
}

//----------------------------------------------------------------------

void SwgCuiLfg::loadTable()
{
	NodeVector topNodes =  LfgDataTable::getTopLevelLfgNodes();

	NodeVector::iterator iter = topNodes.begin();

	for (; iter != topNodes.end(); ++iter)
	{									 
		UIComposite *newpage = NULL;
		
		if (m_externalCriteriaOnly)
		{
			if (!(*iter)->hasAnyExternalAttributeLeafNodeDescendants)
				continue;
		}

		if ((*iter)->isLeafNode())
		{
			if ((*iter)->isMinMaxNode())
			{
				newpage = static_cast<UIComposite *> (m_sampleMinMax->DuplicateObject());
			}
			else
				newpage = static_cast<UIComposite *> (m_sampleChild->DuplicateObject());
		}
		else
			newpage = static_cast<UIComposite *> (m_samplePage->DuplicateObject());

		if (newpage)
			populateBranchPage(*iter, newpage);
 	}
	
	saveOrLoadBackup(false);
	
	Os::createDirectories (defaultPath.c_str());
	updateCombobox();
}

//----------------------------------------------------------------------

void SwgCuiLfg::populateBranchPage (LfgDataTable::LfgNode const * node, UIPage * page)
{
	if (node)
	{
		m_criteriaPage->AddChild(page);
		m_criteriaPage->MoveChild(page, UIBaseObject::Bottom);
		
		page->SetVisible(true);
		
		if (!node->isLeafNode())
		{
			populateChildrenPage(node, page, 1);
			wrapParentToChildren(page);
		}

		initPageDefaults(page, node);
	}
}

void SwgCuiLfg::populateChildrenPage (LfgDataTable::LfgNode const * node, UIPage * parentPage, int tier)
{
	UIComposite * containerPage = static_cast<UIComposite *> (parentPage->GetChild("ContainerPage"));

	NodeVector children =  node->children;

	NodeVector::iterator iter = children.begin();

	for (; iter != children.end(); ++iter)
	{
		UIComposite *newpage = NULL;
		const bool isLeaf = (*iter)->isLeafNode();
		
		if (isLeaf)
		{
			if (m_externalCriteriaOnly)
			{
				if ((*iter)->internalAttribute)
					continue;
			}
			
			if ((*iter)->isMinMaxNode())
			{
				newpage = static_cast<UIComposite *> (m_sampleMinMax->DuplicateObject());
		  	}
			else
				newpage = static_cast<UIComposite *> (m_sampleChild->DuplicateObject());
		}
		else
		{
			if (m_externalCriteriaOnly)
			{
				if (!(*iter)->hasAnyExternalAttributeLeafNodeDescendants)
					continue;
			}
			newpage = static_cast<UIComposite *> (m_samplePage->DuplicateObject());
		}
		
		if (newpage)
		{
			newpage->SetVisible(true);

			UIPage * offsetPage = getOffsetPage(newpage);

			if (offsetPage)
			{
				UIPoint loc = offsetPage->GetLocation();
				loc.x += childOffset * tier;
				offsetPage->SetLocation(loc);

				initPageDefaults(newpage, *iter);
			}

			if (containerPage)
			{
				containerPage->AddChild(newpage);
				containerPage->MoveChild(newpage, UIBaseObject::Bottom);
				containerPage->SetVisible(false);
				containerPage->SetPackDirty(true);

				wrapParentToChildren(containerPage);
			}
		}
		
		if (!isLeaf)
			populateChildrenPage(*iter, newpage, tier + 1);
	}
}

//----------------------------------------------------------------------

void SwgCuiLfg::linkPageItems(UIPage * const page, std::string const & name)
{
	UIComposite * containerPage = static_cast<UIComposite *> (page->GetChild("ContainerPage"));
	
	Unicode::String localName = StringId("lfg_n", name).localize();

	if (containerPage)
	{
		UIWidget * widget = NULL;

		widget = getWidget(page, buttonTree);

		if (widget)
		{
			s_buttonLinks[widget] = containerPage;
			widget->AddCallback(this);
		}

		widget = getWidget(page, buttonAll);

		if (widget)
		{
			s_buttonLinks[widget] = containerPage;
			widget->AddCallback(this);
			widget->SetLocalTooltip(widget->GetLocalTooltip() + localName);			
		}

		widget = getWidget(page, buttonNone);

		if (widget)
		{
			s_buttonLinks[widget] = containerPage;
			widget->AddCallback(this);
			widget->SetLocalTooltip(widget->GetLocalTooltip() + localName);
		}
	}
	
	UIWidget * widget = getWidget(page, checkChosen);

	if (widget)
	{
		s_checkLinks[widget] = page;
		widget->AddCallback(this);
	}
	
	UIWidget * widgetAny = NULL;
	UIWidget * widgetAll = NULL;
	
	widgetAny = getCheckAnyAll(page, checkAny);
	widgetAll = getCheckAnyAll(page, checkAll);

	if (widgetAny && widgetAll)
	{
		s_checkAnyAll.insert(std::make_pair(widgetAny, widgetAll));
		s_checkAnyAll.insert(std::make_pair(widgetAll, widgetAny));
		widgetAny->AddCallback(this);
		widgetAll->AddCallback(this);

		widgetAny->SetLocalTooltip(widgetAny->GetLocalTooltip() + localName);
		widgetAll->SetLocalTooltip(widgetAll->GetLocalTooltip() + localName);
	}
}

//----------------------------------------------------------------------

void SwgCuiLfg::initPageDefaults(UIPage * const page, LfgDataTable::LfgNode const * node)
{
	if (!page || !node)
	   return;
	
	(*m_nodeLinks)[page] = node;
	(*m_stringLinks)[node->name] = page;
	
	linkPageItems(page, node->name);

	UIPage * offsetPage = getOffsetPage(page);

	if (!offsetPage)
	   return;
	
	StringId toolTipId = StringId("lfg_d", node->name);

	Unicode::String tooltip;   
	
	if (toolTipId.localize(tooltip))
		offsetPage->SetLocalTooltip(tooltip);

	if (!node->isLeafNode())
	{
		setName(offsetPage, node->name, colorDarkYellow);

		MatchCondition defaultCondition = node->defaultMatchCondition;

		if (defaultCondition == LfgDataTable::DMCT_NA)
		{
			setCheckAnyAll(page, checkAny, false, false, true, true);
			setCheckAnyAll(page, checkAll, false, false, true, true);
		}
		else if (defaultCondition == LfgDataTable::DMCT_Any)
		{
			setCheckAnyAll(page, checkAny, true, false,  false, true);
			setCheckAnyAll(page, checkAll, false, false, false, true);
		}
		else if (defaultCondition == LfgDataTable::DMCT_AnyOnly)
		{
			setCheckAnyAll(page, checkAny, true, false,  true, true);
			setCheckAnyAll(page, checkAll, false, false,  true, true);
		}
		else if (defaultCondition == LfgDataTable::DMCT_All)
		{
			setCheckAnyAll(page, checkAll, true, false,  false, true);
			setCheckAnyAll(page, checkAny, false, false, false, true);
		}
		else if (defaultCondition == LfgDataTable::DMCT_AllOnly)
		{
			setCheckAnyAll(page, checkAll, true, false,  true, true);
			setCheckAnyAll(page, checkAny, false, false,  true, true);
		}
	}
	else
	{
		setName(offsetPage, node->name, UIColor::white);
		
		if (node->isMinMaxNode())
			updateMinMaxPage(page, false, true);	
	}
}

//----------------------------------------------------------------------

void SwgCuiLfg::updateMinMaxPage (UIPage * const  page, const bool enable, const bool setDefaults, const bool useParams, const int minVal, const int maxVal)
{

	LfgDataTable::LfgNode const * node = (*m_nodeLinks)[page];

	if (!node || !node->isMinMaxNode())
		return;

	UIPage * minMaxPage = NULL;
	UITextbox * minMaxTextBox = NULL;
	UIString textValue;
	FormattedString<16> textBuffer;

	minMaxPage = static_cast<UIPage *> (getWidget(page, std::string("minbox")));
	if (minMaxPage)
	{
		minMaxTextBox = static_cast<UITextbox *> (minMaxPage->GetChild("min"));
		if (minMaxTextBox)
		{
			if (setDefaults)
			{
				textValue = Unicode::narrowToWide (textBuffer.sprintf("%d", node->minValue));
				minMaxTextBox->SetText(textValue);
			}
			else if (useParams)
			{
				if (minVal >= node->minValue && minVal <= node->maxValue )
				{
					textValue = Unicode::narrowToWide (textBuffer.sprintf("%d", minVal));
					minMaxTextBox->SetText(textValue);
				}
			}

			minMaxPage->SetPalShade(enable ? 1.0f : disabledShade);
			minMaxTextBox->SetPalShade(enable ? 1.0f : disabledShade);
			minMaxTextBox->SetEditable(enable);
		}
	}

	minMaxPage = static_cast<UIPage *> (getWidget(page, std::string("maxbox")));
	if (minMaxPage)
	{
		minMaxTextBox = static_cast<UITextbox *> (minMaxPage->GetChild("max"));
		if (minMaxTextBox)
		{
			if (setDefaults)
			{
				textValue = Unicode::narrowToWide (textBuffer.sprintf("%d", node->maxValue));
				minMaxTextBox->SetText(textValue);
			}
			else if (useParams)
			{
				if (maxVal >= node->minValue && maxVal >= node->minValue )
				{
					textValue = Unicode::narrowToWide (textBuffer.sprintf("%d", maxVal));
					minMaxTextBox->SetText(textValue);
				}
			}

			minMaxPage->SetPalShade(enable ? 1.0f : disabledShade);
			minMaxTextBox->SetPalShade(enable ? 1.0f : disabledShade);
			minMaxTextBox->SetEditable(enable);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiLfg::setCheckAnyAll(UIPage * const page, const std::string & widgetName, const bool check, const bool ignoreCheck, const bool disable, const bool hide)
{
	UICheckbox * checkBox = getCheckAnyAll(page, widgetName);

	if (checkBox)
	{
		if (!ignoreCheck)
			checkBox->SetChecked(check);

		if (disable)
		{
			checkBox->SetPalShade(disabledShade);
			checkBox->SetEnabled(false);
		}
		
		checkBox->SetVisible(!hide);
	}
}

//----------------------------------------------------------------------

void SwgCuiLfg::OnCheckboxSet(UIWidget * context)
{
	if (!context)
		return;
	
	if (context->GetName() == checkAll || context->GetName() == checkAny)
	{
		UICheckbox * brotherCheck = static_cast<UICheckbox *> (s_checkAnyAll[context]);

		if (brotherCheck)
			brotherCheck->SetChecked(false, false);
	}
	else if (context->GetName() == checkChosen)
	{
		UIPage * page = s_checkLinks[context];

		if (!page)
			return;

		updateMinMaxPage(page, true);

		page = getParentBranch(page);

		if (!page)
			return;

		calculateBranchColor(page);
		updateParentStates(page);
			
	}
}

//----------------------------------------------------------------------

void SwgCuiLfg::OnCheckboxUnset(UIWidget *context)
{
	if (!context)
		return;

	if (context->GetName() == checkAll || context->GetName() == checkAny)
	{
		UICheckbox * brotherCheck = static_cast<UICheckbox *> (s_checkAnyAll[context]);

		if (brotherCheck)
			brotherCheck->SetChecked(true, false);
	}
	else if (context->GetName() == checkChosen)
	{
		UIPage * page = s_checkLinks[context];

		if (!page)
			return;
		
		updateMinMaxPage(page, false);

		page = getParentBranch(page);

		if (!page)
			return;

		calculateBranchColor(page);
		updateParentStates(page);
	}
}

//----------------------------------------------------------------------

void SwgCuiLfg::OnButtonPressed(UIWidget *context)
{
	if (!context)
		return;

	UINarrowString buttonName = context->GetName();
		
	if (buttonName == buttonTree)
	{
		UIComposite * page = s_buttonLinks[context];

		if (!page)
			return;

		bool isVisible = page->IsVisible();

		if (isVisible)
			context->SetProperty (UIButton::PropertyName::Style, styleMinus);
		else
			context->SetProperty (UIButton::PropertyName::Style, stylePlus);

		page->SetVisible(!isVisible);

		wrapParentToChildren(page);
		wrapParentPages(page);

		m_criteriaPage->SetPackDirty(true);
	}
	else if (buttonName == buttonNone)
	{
		UIComposite * page = s_buttonLinks[context];

		if (!page)
			return;

		checkAllChildren(page, false);

		UIPage * parent = static_cast<UIPage *> (page->GetParent());

		if (!parent)
			return;

		changeBranchColor(parent, colorDarkYellow, true);
		updateParentStates(parent);
	}
	else if (buttonName == buttonAll)
	{
		UIComposite * page = s_buttonLinks[context];

		if (!page)
			return;

		checkAllChildren(page, true);

		UIPage * parent = static_cast<UIPage *> (page->GetParent());

		if (!parent)
			return;

		changeBranchColor(parent, UIColor::yellow, true);
		updateParentStates(parent);
	}
	else if (buttonName == buttonSave)
	{
		if (m_comboBox)
		{
			Unicode::String selectedName;

			m_comboBox->GetProperty(UIComboBox::PropertyName::SelectedText, selectedName);
						
			Unicode::String::size_type pos = selectedName.find(Unicode::narrowToWide("@"));

			while (pos == 0)
			{
				selectedName.erase(pos, 1);
				pos = selectedName.find(Unicode::narrowToWide("@"));
			}
			
			if (!selectedName.empty() && selectedName != Unicode::narrowToWide("New File"))
			{
				saveCriteria(std::string (defaultPath + '/' + Unicode::wideToNarrow(selectedName) + ".swg").c_str());
			}

			updateCombobox();
		}
	}
	else if (buttonName == buttonLoad)
	{
		if (m_comboBox)
		{
			Unicode::String selectedName;

			m_comboBox->GetProperty(UIComboBox::PropertyName::SelectedText, selectedName);

			if (!selectedName.empty() && selectedName != Unicode::narrowToWide("New File"))
				loadCriteria(std::string (defaultPath + '/' + Unicode::wideToNarrow(selectedName) + ".swg").c_str());
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiLfg::updateParentStates (UIPage * const page)
{
	if (!page)
		return;

	UIPage * parent = static_cast <UIPage *>(page->GetParent());

	if (!parent)
		return;

	parent = static_cast <UIPage *>(parent->GetParent());

	if (!parent)
		return;

	UIComposite * containerPage = static_cast<UIComposite *> (parent->GetChild("ContainerPage"));

	if (!containerPage)
		return;

	UIBaseObject::UIObjectList children;

	containerPage->GetChildren(children);

	UIBaseObject::UIObjectList::iterator iter = children.begin();

	int totalBranches = 0;
	int emptyBranches = 0;
	int fullBranches  = 0;

	for (; iter != children.end(); ++iter)
	{
		UIBaseObject * child = *iter;

		if (child->IsA(TUIPage))
		{
			UIPage * childPage = static_cast<UIPage *> (child);

			if (childPage->GetName() == pageBranch)
			{
				++totalBranches;

				switch(getCriteriaState(childPage))
				{
				case CS_NONE:
					++emptyBranches;
					break;
				
				case CS_ALL:
					++fullBranches;
				    break;
				
				default:
				    break;
				}

			}
			else if (childPage->GetName() == pageLeaf || childPage->GetName() == pageMinMax)
			{
				UICheckbox * checkBox = static_cast<UICheckbox *> (getWidget(childPage, checkChosen));

				if(!checkBox)
					continue;

				++totalBranches;

				if(checkBox->IsChecked())
					++fullBranches;
				else
					++emptyBranches;
			}
		}
	}

	children.clear();
	
	if (fullBranches == totalBranches)
	{
		changeBranchColor(parent, UIColor::yellow);
	}
	else if (emptyBranches == totalBranches)
	{
		changeBranchColor(parent, colorDarkYellow);
	}
	else
	{
		changeBranchColor(parent, colorManilla);
	}

	updateParentStates(parent);
 }

//----------------------------------------------------------------------

void SwgCuiLfg::calculateBranchColor (UIPage  * const  page)
{
	switch(getCriteriaState(page))
	{
	case CS_NONE:
		changeBranchColor(page, colorDarkYellow);
		break;
	
	case CS_SOME:
		changeBranchColor(page, colorManilla);
		break;
	
	case CS_ALL:
		changeBranchColor(page, UIColor::yellow);
		break;
	
	default:
		break;
	}
}

//----------------------------------------------------------------------

void SwgCuiLfg::updateBranchStateByName(const std::string & name)
{
	UIPage * const page = (*m_stringLinks)[name];

	if (!page)
		return;

	calculateBranchColor(page);
}

//----------------------------------------------------------------------

void SwgCuiLfg::disableWindow ()
{
	getPage().SetPalShade(disabledShade);
	getPage().SetEnabled(false);
}

//----------------------------------------------------------------------

void SwgCuiLfg::enableWindow  ()
{
	getPage().SetPalShade(1.0f);
	getPage().SetEnabled(true);
}	

//----------------------------------------------------------------------

SwgCuiLfg::CriteriaState SwgCuiLfg::getCriteriaState (UIPage const *  page)
{
	
	if (!page)
		return CS_NONE;
		
	UIComposite * containerPage = static_cast<UIComposite *> (page->GetChild("ContainerPage"));

	if (!containerPage)
		return CS_NONE;
		
	UIBaseObject::UIObjectList children;

	containerPage->GetChildren(children);

	UIBaseObject::UIObjectList::iterator iter = children.begin();

	int totalChecks = 0;
	int checked = 0;

	for (; iter != children.end(); ++iter)
	{
		UIBaseObject * child = *iter;

		if (child->IsA(TUIPage))
		{
			UIPage * childPage = static_cast<UIPage *> (child);

			if (childPage->GetName() == pageLeaf || childPage->GetName() == pageMinMax)
			{
				UICheckbox * checkBox = static_cast<UICheckbox *> (getWidget(childPage, checkChosen));

				if(!checkBox)
					continue;

				++totalChecks;

				if(checkBox->IsChecked())
					++checked;
			}
			else if (childPage->GetName() == pageBranch)
			{
				++totalChecks;
				switch(getCriteriaState(childPage))
				{
				case CS_ALL:
					++checked;
					break;
				
				case CS_SOME:
					++checked;
				    break;
				
				default:
				    break;
				}
			}
		}
	}

	children.clear();

	if (checked == 0)
	{
		return CS_NONE;
	}
	else if (checked == totalChecks)
	{
		return CS_ALL;
	}
	else
	{
		return CS_SOME;
	}

}

//----------------------------------------------------------------------

void SwgCuiLfg::changeBranchColor (UIPage * const page, const UIColor & color, const bool changeChildren)
{
	if (!page)
		return;

	UIPage * offsetPage = getOffsetPage(page);

	if (!offsetPage)
		return;

	UIText * text = static_cast<UIText *> (offsetPage->GetChild("name"));

	if (!text)
		return;
		
	text->SetTextColor(color);

	bool const notVisible = (color == colorDarkYellow);
		
	const LfgDataTable::LfgNode * node = (*m_nodeLinks)[page];

	if (node)
	{
		MatchCondition defaultCondition = node->defaultMatchCondition;

		if (defaultCondition == LfgDataTable::DMCT_Any)
		{
			setCheckAnyAll(page, checkAny, true, true, false, notVisible);
			setCheckAnyAll(page, checkAll, true, true, false, notVisible);
		}
		else if (defaultCondition == LfgDataTable::DMCT_AnyOnly)
		{
			setCheckAnyAll(page, checkAny, true, true, true, notVisible);
		}
		else if (defaultCondition == LfgDataTable::DMCT_All)
		{
			setCheckAnyAll(page, checkAll, true, true, false, notVisible);
			setCheckAnyAll(page, checkAny, true, true, false, notVisible);
		}
		else if (defaultCondition == LfgDataTable::DMCT_AllOnly)
		{
			setCheckAnyAll(page, checkAll, true, true, true, notVisible);
		}
	}

	if (changeChildren)
	{
		UIComposite * containerPage = static_cast<UIComposite *> (page->GetChild("ContainerPage"));

		if (!containerPage)
			return;

		UIBaseObject::UIObjectList children;
		
		containerPage->GetChildren(children);

		UIBaseObject::UIObjectList::iterator iter = children.begin();
		

		for (; iter != children.end(); ++iter)
		{
			UIBaseObject * child = *iter;

			if (child->IsA(TUIPage))
			{
				UIPage * childPage = static_cast<UIPage *> (child);

				if (childPage->GetName() == pageBranch)
				{
					changeBranchColor(childPage, color, changeChildren);
				}
			}
		}

		children.clear();
 	}
}

//----------------------------------------------------------------------

void SwgCuiLfg::checkAllChildren(UIPage * const page, const bool check)
{
	if (!page)
		return;
	
	UIBaseObject::UIObjectList children; 
	page->GetChildren(children);
	UIBaseObject::UIObjectList::iterator iter = children.begin();

	for (; iter != children.end(); ++iter)
	{
		UIBaseObject * child = *iter;

		if (child->IsA(TUIPage))
		{
			UIPage * childPage = static_cast<UIPage *> (child);

			checkAllChildren(childPage, check);

			UICheckbox * checkBox = static_cast<UICheckbox *> (getWidget(childPage, checkChosen));

			if (!checkBox)
				continue;

			checkBox->SetChecked(check, true);
		}
	}

	children.clear();
 }

//----------------------------------------------------------------------

void SwgCuiLfg::setName (UIPage * page, std::string const & name, UIColor const & color)
{
	UIText * text = static_cast<UIText *> (page->GetChild("name"));

	if (!text)
		return;
	
	text->SetText(StringId("lfg_n", name).localize());
	text->SetTextColor(color);
}

//----------------------------------------------------------------------

void SwgCuiLfg::wrapParentPages(UIPage * page)
{
	UIPage * parentPage = static_cast<UIPage *> (page->GetParent());
	
	while (parentPage && parentPage != m_criteriaPage)
	{
		wrapParentToChildren(parentPage);

		parentPage = static_cast<UIPage *> (parentPage->GetParent());
	}
}

//----------------------------------------------------------------------

UICheckbox * SwgCuiLfg::getCheckAnyAll(UIPage const *  page, const std::string & widgetName)
{
	UIPage const * tierPage = static_cast<UIPage *> (page->GetChild("TierPage"));

	if (!tierPage)
		return NULL;

	return static_cast<UICheckbox *> (tierPage->GetChild(widgetName.c_str()));
}

//----------------------------------------------------------------------

UIWidget * SwgCuiLfg::getWidgetByName (std::string const & parentName, std::string const & widgetName)
{
	UIPage const * page = (*m_stringLinks)[parentName];
	if(!page)
		return NULL;

	return getWidget(page, widgetName);
}

//----------------------------------------------------------------------

UIWidget * SwgCuiLfg::getWidget(UIPage const *  page, std::string const & widgetName)
{
	UIPage const * tierPage = static_cast<UIPage *> (page->GetChild("TierPage"));
	
	if (!tierPage)
		return NULL;

	UIPage const * offsetPage = static_cast<UIPage *> (tierPage->GetChild("OffsetPage"));

	if (!offsetPage)
		return NULL;
	
	return static_cast<UIWidget *> (offsetPage->GetChild(widgetName.c_str()));
}

//----------------------------------------------------------------------

UIPage * SwgCuiLfg::getParentBranch(UIPage *  page)
{
	UIPage * container = static_cast<UIPage *>(page->GetParent());

	if (!container)
		return NULL;

	return static_cast<UIPage *>(container->GetParent());
}

//----------------------------------------------------------------------

UIPage * SwgCuiLfg::getOffsetPage (UIPage const * page)
{
	UIPage const * tierPage = static_cast<UIPage *> (page->GetChild("TierPage"));

	if (!tierPage)
		return NULL;	
	
	return static_cast<UIPage *> (tierPage->GetChild("OffsetPage"));;
}

//----------------------------------------------------------------------

void SwgCuiLfg::wrapParentToChildren(UIPage * parentPage)
{
	UIBaseObject::UISmartObjectVector children;
	parentPage->ForcePackChildren();
	parentPage->GetVisibleChildren(children);

	UIBaseObject::UISmartObjectVector::iterator iter = children.begin();
	
	UISize maxSize(0,0);
	
	for (; iter != children.end(); ++iter)
	{
		UIBaseObject * const obj = *iter;

		if (obj->IsA(TUIWidget))
		{				
			UIWidget const * child = static_cast<UIWidget *> (obj);

			if (!child->IsVisible())
				continue;

			UIPoint loc = child->GetLocation();
			loc += child->GetSize();
			
			if (loc.x > maxSize.x)
				maxSize.x = loc.x;
			
			if (loc.y > maxSize.y)
				maxSize.y = loc.y;
		}
	}
	
	maxSize.y += containerPadding;

	parentPage->SetSize(maxSize);
}

//----------------------------------------------------------------------

void SwgCuiLfg::resetAllCriteria ()
{
	if (!m_criteriaPage)
		return;
	
	checkAllChildren(m_criteriaPage, false);

	UIBaseObject::UIObjectList children;

	m_criteriaPage->GetChildren(children);

	UIBaseObject::UIObjectList::iterator iter = children.begin();


	for (; iter != children.end(); ++iter)
	{
		UIBaseObject * child = *iter;

		if (child->IsA(TUIPage))
		{
			UIPage * childPage = static_cast<UIPage *> (child);

			if (childPage->GetName() == pageBranch)
			{
				changeBranchColor(childPage, colorDarkYellow, true);
			}
		}
	}

	children.clear();
}

//----------------------------------------------------------------------

void SwgCuiLfg::loadCriteria(char const * filename)
{
	if (!filename)
		return;	

	resetAllCriteria();

	std::ifstream inputFile;
	inputFile.open(filename, std::ios_base::in);

	char theType = 0;
	char const delim = '\n';
	int const bufferSize = 128;

	if (inputFile.is_open())
	{
		while (inputFile.good())
		{
			theType = static_cast <char> (inputFile.get());

			if (!inputFile.good())
				break;

			if (theType ==  '+')
			{
				char name[bufferSize];
				inputFile.getline(name, bufferSize, ' ');

				if (!inputFile.good())
					break;

				UIPage * page = (*m_stringLinks)[std::string(name)];

				if (!page)
					continue;

				memset(name, 0, bufferSize);
				inputFile.getline(name, bufferSize, delim);

				if (!inputFile.good())
					break;

				if (!strcmp(name, "Any"))
				{
					UICheckbox * check = getCheckAnyAll(page, checkAny);

					if (!check)
						continue;

					check->SetChecked(true);

				}
				else if (!strcmp(name, "All"))
				{
					UICheckbox * check = getCheckAnyAll(page, checkAll);

					if (!check)
						continue;

					check->SetChecked(true);
				}

			}
			else if (theType ==  '-')
			{
				char name[bufferSize];
				inputFile.getline(name, bufferSize, delim);
				
				if (!inputFile.good())
					break;

				UIPage * page = (*m_stringLinks)[std::string(name)];

				if (!page)
					continue;
								
				UICheckbox * check = static_cast<UICheckbox *> (getWidget(page, checkChosen));

				if (!check)
					continue;

				check->SetChecked(true);								
			}
			else if (theType ==  '#')
			{
				char name[bufferSize];
				inputFile.getline(name, bufferSize, ' ');

				if (!inputFile.good())
					break;

				UIPage * page = (*m_stringLinks)[std::string(name)];
				
				if (!page)
					continue;

				UICheckbox * check = static_cast<UICheckbox *> (getWidget(page, checkChosen));

				if (!check)
					continue;

				check->SetChecked(true);

				LfgDataTable::LfgNode const * node = (*m_nodeLinks)[page];

				if (!node)
					continue;

				if (node->isMinMaxNode())
				{
					memset(name, 0, bufferSize);
					inputFile.getline(name, bufferSize, ' ');

					if (!inputFile.good())
						break;
					
					int const minimum = atoi(name);

					memset(name, 0, bufferSize);
					inputFile.getline(name, bufferSize, delim);

					if (!inputFile.good())
						break;

					int const maximum = atoi(name);

					updateMinMaxPage(page, true, false, true, minimum, maximum);
				}
			}
		}
	}

	inputFile.close();
}

//----------------------------------------------------------------------

void SwgCuiLfg::saveCriteria(char const * filename)
{
	if (!filename)
		return;

	std::ofstream outputFile;
	outputFile.open(filename, std::ios_base::out | std::ios_base::trunc);
	
	if (outputFile.is_open())
	{
		PageMap::iterator iter = (*m_nodeLinks).begin();

		for (; iter != (*m_nodeLinks).end(); ++iter)
		{
			LfgDataTable::LfgNode const * node = (*iter).second;
			UIPage * const page = (*iter).first;

			if (node && page && outputFile.good())
			{
				if (node->isLeafNode())
				{
					if (node->isMinMaxNode())
					{
						UICheckbox * checkBox = static_cast<UICheckbox *> (getWidget(page, checkChosen));

						if (!checkBox)
							continue;

						int min = 0;
						int max = 0;

						getPageMinMaxValues(page, min, max);

						if (checkBox->IsChecked())
							outputFile << '#' << node->name << ' '  << min << ' ' << max << "\n";
					}
					else
					{
						UICheckbox * checkBox = static_cast<UICheckbox *> (getWidget(page, checkChosen));

						if (!checkBox)
							continue;

						if (checkBox->IsChecked())
							outputFile << '-' << node->name << "\n";
					}
				}
				else
				{
					MatchCondition nodeCondition = node->defaultMatchCondition;
					if (nodeCondition == LfgDataTable::DMCT_Any || nodeCondition == LfgDataTable::DMCT_All)
					{
						UICheckbox * checkBox = getCheckAnyAll(page, checkAny);

						if (!checkBox || !checkBox->IsVisible())
							continue;

						if (checkBox->IsChecked())
						{
							outputFile << '+' << node->name << " Any" << "\n";
						}
						else
						{
							outputFile << '+' << node->name << " All" << '\n';
						}
					}
				}
			}
		}
	}

	outputFile.close();
}

//----------------------------------------------------------------------

void SwgCuiLfg::getAllSearchCriteria (BitArray & criteriaBits)
{
	PageMap::iterator iter = (*m_nodeLinks).begin();

	for (; iter != (*m_nodeLinks).end(); ++iter)
	{
		LfgDataTable::LfgNode const * node = (*iter).second;
		UIPage * const page = (*iter).first;

		if (!node || !page)
			continue;

		if (node->isLeafNode())
		{
			UICheckbox * checkBox = static_cast<UICheckbox *> (getWidget(page, checkChosen));

			if (!checkBox || !checkBox->IsChecked())
				continue;

			if (node->isMinMaxNode())
			{
				int min = 0;
				int max = 0;

				getPageMinMaxValues(page, min, max);

				LfgDataTable::setLowHighValue(*node, min, max, criteriaBits);
			}
			else
			{
				LfgDataTable::setBit(*node, criteriaBits);
			}
		}
		else
		{
			CriteriaState pageState = getCriteriaState(page);
			
			if (pageState == CS_NONE)
				continue;

			UICheckbox * checkBox = getCheckAnyAll(page, checkAny);

			if (!checkBox)
				continue;

			bool const isAnyChecked = checkBox->IsChecked();

			LfgDataTable::setAnyOrAllBit(*node, isAnyChecked, criteriaBits);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiLfg::sendInterestsToServer ()
{
	if (!m_externalCriteriaOnly)
		return;

	BitArray interests;

	PageMap::iterator iter = (*m_nodeLinks).begin();

	for (; iter != (*m_nodeLinks).end(); ++iter)
	{
		LfgDataTable::LfgNode const * node = (*iter).second;
		UIPage * page = (*iter).first;

		if (!node || !page || !node->isLeafNode())
			continue;

		UICheckbox * checkBox = static_cast<UICheckbox *> (getWidget(page, checkChosen));

		if (!checkBox || !checkBox->IsChecked())
			continue;

		if (node->isMinMaxNode())
		{
			int min = 0;
			int max = 0;
			
			getPageMinMaxValues(page, min, max);

			LfgDataTable::setLowHighValue(*node, min, max, interests);
		}
		else
		{
			LfgDataTable::setBit(*node, interests);
		}
	}

	GenericValueTypeMessage<BitArray> const msg("SetLfgInterests", interests);

	GameNetwork::send(msg, true);
}

//----------------------------------------------------------------------

void SwgCuiLfg::getPageMinMaxValues (UIPage * page, int & min, int & max)
{
	if (!page)
		return;

	LfgDataTable::LfgNode const * node = (*m_nodeLinks)[page];

	if (!node || !node->isMinMaxNode())
		return;

	UIPage * minMaxPage = NULL;
	UITextbox * minMaxTextBox = NULL;
	UIString textValue;
	FormattedString<16> textBuffer;

	minMaxPage = static_cast<UIPage *> (getWidget(page, std::string("minbox")));
	if (minMaxPage)
	{
		minMaxTextBox = static_cast<UITextbox *> (minMaxPage->GetChild("min"));
		if (minMaxTextBox)
		{
			min = minMaxTextBox->GetNumericIntegerValue();

			if (min < node->minValue || min > node->maxValue )
			{
				min = node->minValue;
				textValue = Unicode::narrowToWide (textBuffer.sprintf("%d", min));
				minMaxTextBox->SetText(textValue);
			}
		}
	}

	minMaxPage = static_cast<UIPage *> (getWidget(page, std::string("maxbox")));
	if (minMaxPage)
	{
		minMaxTextBox = static_cast<UITextbox *> (minMaxPage->GetChild("max"));
		if (minMaxTextBox)
		{
			max = minMaxTextBox->GetNumericIntegerValue();

			if (max < node->minValue || max > node->maxValue )
			{
				max = node->maxValue;
				textValue = Unicode::narrowToWide (textBuffer.sprintf("%d", max));
				minMaxTextBox->SetText(textValue);
			}
		}
	}

	if (min > max)
	{
		std::swap(min, max);
		updateMinMaxPage(page, true, false, true, min, max);
	}
}

//----------------------------------------------------------------------

void SwgCuiLfg::saveOrLoadBackup (bool const shouldSave)
{
	std::string filename;

	getPlayerFilePath(filename);

	if (filename.empty())
		return;
		
	if (shouldSave)
		Os::createDirectories (filename.c_str ());

	if (m_externalCriteriaOnly)
		filename += "/Interests.swg";
	else
		filename += "/Criteria.swg";

	if (shouldSave)
		saveCriteria(filename.c_str());
	else
		loadCriteria(filename.c_str());
}

//----------------------------------------------------------------------

void SwgCuiLfg::sendSavedInterests ()
{
	BitArray  interests;

	std::string filename;

	getPlayerFilePath(filename);

	if (filename.empty())
		return;

	filename += "/Interests.swg"; 

	std::ifstream inputFile;
	inputFile.open(filename.c_str(), std::ios_base::in);

	char theType = 0;
	char const delim = '\n';
	int const bufferSize = 128;

	if (inputFile.is_open())
	{
		while (inputFile.good())
		{
			theType = static_cast <char> (inputFile.get());

			if (!inputFile.good())
				break;

			if (theType ==  '-')
			{
				char name[bufferSize];
				inputFile.getline(name, bufferSize, delim);

				if (!inputFile.good())
					break;
				
				LfgDataTable::LfgNode const * node = LfgDataTable::getLfgNodeByName(name);

				if (node && !node->internalAttribute)
					LfgDataTable::setBit(*node, interests);							
			}
			else if (theType ==  '#')
			{
				char name[bufferSize];
				inputFile.getline(name, bufferSize, ' ');

				if (!inputFile.good())
					break;

				LfgDataTable::LfgNode const * node = LfgDataTable::getLfgNodeByName(name);				

				if (!node)
					continue;

				if (node->isMinMaxNode())
				{
					memset(name, 0, bufferSize);
					inputFile.getline(name, bufferSize, ' ');

					if (!inputFile.good())
						break;

					int minimum = atoi(name);

					memset(name, 0, bufferSize);
					inputFile.getline(name, bufferSize, delim);

					if (!inputFile.good())
						break;

					int maximum = atoi(name);

					errorCheckMinMax(node, minimum, maximum);

					LfgDataTable::setLowHighValue(*node, minimum, maximum, interests);
				}
			}
		}
	}

	inputFile.close();

	GenericValueTypeMessage<BitArray> const msg("SetLfgInterests", interests);

	GameNetwork::send(msg, true);
}

//----------------------------------------------------------------------

void SwgCuiLfg::performActivate   ()
{
}

//----------------------------------------------------------------------

void SwgCuiLfg::performDeactivate  ()
{
}

//----------------------------------------------------------------------

SwgCuiLfg::~SwgCuiLfg ()
{
	s_buttonLinks.clear();
	s_checkAnyAll.clear();
	s_checkLinks.clear();
	(*m_nodeLinks).clear();
	(*m_stringLinks).clear();

	delete m_stringLinks;
	delete m_nodeLinks;

	m_sampleMinMax = NULL;
	m_samplePage = NULL;
	m_criteriaPage = NULL;
	m_sampleChild = NULL;
	m_comboBox = NULL;
}

//----------------------------------------------------------------------

void SwgCuiLfg::updateCombobox()
{
	if (!m_comboBox)
		return;

	m_comboBox->Clear();
	m_comboBox->AddItem(Unicode::narrowToWide("New File"), "New File");

	m_comboBox->SetSelectedIndex(-1);

	std::string playerPath = defaultPath + "/*";

	WIN32_FIND_DATA findFileData;
	HANDLE fileHandle;

	fileHandle = FindFirstFile(playerPath.c_str(), &findFileData);

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		do 
		{
			bool const isFileValid = (!(findFileData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN)));
			
			if (isFileValid)
			{
				std::string fileName = findFileData.cFileName;
				std::string::size_type pos = fileName.find(".swg"); 
				if (pos != std::string::npos)
				{
					fileName.erase(pos, pos + 3);
					m_comboBox->AddItem(Unicode::narrowToWide(fileName), findFileData.cFileName);
				}
			}			
		} while(FindNextFile( fileHandle, &findFileData));
		
		FindClose(fileHandle);
	}
}

//----------------------------------------------------------------------

void SwgCuiLfg::OnGenericSelectionChanged(UIWidget * context)
{
	if (m_comboBox && context == m_comboBox)
	{					
		if (m_comboBox->GetSelectedIndex() == 0)
		{
			m_comboBox->SetPropertyBoolean(UIComboBox::PropertyName::Editable, true);

			UITextbox * textBox = static_cast<UITextbox *> (m_comboBox->GetChild("ComboTextbox"));

			if (textBox)
			{
				textBox->SetLocalText(UIString(Unicode::narrowToWide("New File")));
				textBox->SelectAll();
				textBox->MoveCaratToEndOfLine();
				textBox->SetSelected(true);
				textBox->SetFocus();
			}
		}
		else
		{
			UITextbox * textBox = static_cast<UITextbox *> (m_comboBox->GetChild("ComboTextbox"));

			if (textBox)
				textBox->ClearSelection();

			m_comboBox->SetPropertyBoolean(UIComboBox::PropertyName::Editable, false);
		}
	}			
}

//----------------------------------------------------------------------

void SwgCuiLfgNamespace::errorCheckMinMax(LfgDataTable::LfgNode const * node, int & min, int & max)
{
	if (min < node->minValue || min > node->maxValue )
		min = node->minValue;

	if (max < node->minValue || max > node->maxValue )
		max = node->maxValue;

	if (min > max)
		std::swap(min, max);	
}

//----------------------------------------------------------------------

void SwgCuiLfgNamespace::getPlayerFilePath (std::string & path)
{
	std::string     loginId;
	std::string     clusterName;
	Unicode::String playerName;
	NetworkId       playerId;

	if (!Game::getPlayerPath      (loginId, clusterName, playerName, playerId))
		return;

	path = std::string ("profiles/") + loginId + "/" + clusterName + "/" + playerId.getValueString ();
}
//======================================================================
