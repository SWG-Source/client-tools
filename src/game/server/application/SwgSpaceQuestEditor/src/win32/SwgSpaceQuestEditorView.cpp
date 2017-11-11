// ======================================================================
//
// SwgSpaceQuestEditorView.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceQuestEditor.h"
#include "SwgSpaceQuestEditorView.h"

#include "Configuration.h"
#include "DialogStringIdList.h"
#include "Resource.h"
#include "SwgSpaceQuestEditorDoc.h"

// ======================================================================

namespace SwgSpaceQuestEditorViewNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - =

	int getPropertyType(Configuration::MissionTemplate::PropertyTemplate const & propertyTemplate)
	{
		switch (propertyTemplate.m_propertyType)
		{
		case Configuration::MissionTemplate::PropertyTemplate::PT_spaceMobile:
			return ID_PROPERTY_TEXT_SPACEMOBILE;

		case Configuration::MissionTemplate::PropertyTemplate::PT_spaceMobileList:
			return ID_PROPERTY_TEXT_SPACEMOBILELIST;

		case Configuration::MissionTemplate::PropertyTemplate::PT_navPoint:
			return ID_PROPERTY_TEXT_NAVPOINT;

		case Configuration::MissionTemplate::PropertyTemplate::PT_navPointList:
			return ID_PROPERTY_TEXT_NAVPOINTLIST;

		case Configuration::MissionTemplate::PropertyTemplate::PT_questName:
			return ID_PROPERTY_TEXT_QUEST;

		case Configuration::MissionTemplate::PropertyTemplate::PT_objectTemplate:
			return ID_PROPERTY_TEXT_OBJECTTEMPLATE;

		case Configuration::MissionTemplate::PropertyTemplate::PT_spawner:
			return ID_PROPERTY_TEXT_SPAWNER;

		case Configuration::MissionTemplate::PropertyTemplate::PT_spawnerList:
			return ID_PROPERTY_TEXT_SPAWNERLIST;

		case Configuration::MissionTemplate::PropertyTemplate::PT_spaceFaction:
			return ID_PROPERTY_COMBO_LIST;

		case Configuration::MissionTemplate::PropertyTemplate::PT_spaceZone:
			return ID_PROPERTY_COMBO_LIST;

		case Configuration::MissionTemplate::PropertyTemplate::PT_enumList:
			return ID_PROPERTY_COMBO_LIST;

		case Configuration::MissionTemplate::PropertyTemplate::PT_integer:
			return ID_PROPERTY_INTEGER;

		case Configuration::MissionTemplate::PropertyTemplate::PT_string:
			return ID_PROPERTY_TEXT;

		case Configuration::MissionTemplate::PropertyTemplate::PT_bool:
			return ID_PROPERTY_BOOL;
		
		case Configuration::MissionTemplate::PropertyTemplate::PT_cargo:
			return ID_PROPERTY_TEXT_CARGO;
		}

		return ID_PROPERTY_STATIC;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - =

	CString const getPropertyData(Configuration::MissionTemplate::PropertyTemplate const & propertyTemplate, SpaceQuest const & spaceQuest)
	{
		switch (propertyTemplate.m_propertyType)
		{
		case Configuration::MissionTemplate::PropertyTemplate::PT_spaceMobile:
			return spaceQuest.getProperty(propertyTemplate.m_columnName);

		case Configuration::MissionTemplate::PropertyTemplate::PT_spaceMobileList:
			return spaceQuest.getProperty(propertyTemplate.m_columnName);

		case Configuration::MissionTemplate::PropertyTemplate::PT_spawner:
			return spaceQuest.getProperty(propertyTemplate.m_columnName);

		case Configuration::MissionTemplate::PropertyTemplate::PT_spawnerList:
			return spaceQuest.getProperty(propertyTemplate.m_columnName);

		case Configuration::MissionTemplate::PropertyTemplate::PT_navPoint:
			return spaceQuest.getProperty(propertyTemplate.m_columnName);

		case Configuration::MissionTemplate::PropertyTemplate::PT_navPointList:
			return spaceQuest.getProperty(propertyTemplate.m_columnName);

		case Configuration::MissionTemplate::PropertyTemplate::PT_questName:
			return spaceQuest.getProperty(propertyTemplate.m_columnName);

		case Configuration::MissionTemplate::PropertyTemplate::PT_objectTemplate:
			return spaceQuest.getProperty(propertyTemplate.m_columnName);

		case Configuration::MissionTemplate::PropertyTemplate::PT_spaceFaction:
			return Configuration::getSpaceFactionList();

		case Configuration::MissionTemplate::PropertyTemplate::PT_spaceZone:
			return Configuration::getSpaceZoneList();

		case Configuration::MissionTemplate::PropertyTemplate::PT_enumList:
			{
				CString result;
				Configuration::packString(propertyTemplate.m_propertyDataList, result, '!');
				return result;
			}

		case Configuration::MissionTemplate::PropertyTemplate::PT_integer:
			return spaceQuest.getProperty(propertyTemplate.m_columnName);

		case Configuration::MissionTemplate::PropertyTemplate::PT_string:
			return spaceQuest.getProperty(propertyTemplate.m_columnName);

		case Configuration::MissionTemplate::PropertyTemplate::PT_bool:
			return "false!true";

		case Configuration::MissionTemplate::PropertyTemplate::PT_cargo:
			return spaceQuest.getProperty(propertyTemplate.m_columnName);
		}

		return "ERROR(2): see asommers";
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - =

	int getPropertySelection(Configuration::MissionTemplate::PropertyTemplate const & propertyTemplate, SpaceQuest const & spaceQuest)
	{
		switch (propertyTemplate.m_propertyType)
		{
		case Configuration::MissionTemplate::PropertyTemplate::PT_spaceMobile:
			return 0;

		case Configuration::MissionTemplate::PropertyTemplate::PT_spaceMobileList:
			return 0;

		case Configuration::MissionTemplate::PropertyTemplate::PT_spawner:
			return 0;

		case Configuration::MissionTemplate::PropertyTemplate::PT_spawnerList:
			return 0;

		case Configuration::MissionTemplate::PropertyTemplate::PT_navPoint:
			return 0;

		case Configuration::MissionTemplate::PropertyTemplate::PT_navPointList:
			return 0;

		case Configuration::MissionTemplate::PropertyTemplate::PT_questName:
			return 0;

		case Configuration::MissionTemplate::PropertyTemplate::PT_objectTemplate:
			return 0;

		case Configuration::MissionTemplate::PropertyTemplate::PT_spaceFaction:
			return Configuration::getSpaceFactionIndex(spaceQuest.getProperty(propertyTemplate.m_columnName));

		case Configuration::MissionTemplate::PropertyTemplate::PT_spaceZone:
			return Configuration::getSpaceZoneIndex(spaceQuest.getProperty(propertyTemplate.m_columnName));

		case Configuration::MissionTemplate::PropertyTemplate::PT_enumList:
			{
				StringList::const_iterator iter = std::find(propertyTemplate.m_propertyDataList.begin(), propertyTemplate.m_propertyDataList.end(), spaceQuest.getProperty(propertyTemplate.m_columnName));
				if (iter == propertyTemplate.m_propertyDataList.end())
					return 0;

				return iter - propertyTemplate.m_propertyDataList.begin();
			}

		case Configuration::MissionTemplate::PropertyTemplate::PT_integer:
			return 0;

		case Configuration::MissionTemplate::PropertyTemplate::PT_string:
			return 0;

		case Configuration::MissionTemplate::PropertyTemplate::PT_bool:
			{
				CString const property = spaceQuest.getProperty(propertyTemplate.m_columnName);
				if (property.IsEmpty())
					return 0;

				return property[0] == '0' ? 0 : 1;
			}

		case Configuration::MissionTemplate::PropertyTemplate::PT_cargo:
			return 0;
		}

		return 0;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - =
}

using namespace SwgSpaceQuestEditorViewNamespace;

// ======================================================================

IMPLEMENT_DYNCREATE(SwgSpaceQuestEditorView, CView)

BEGIN_MESSAGE_MAP(SwgSpaceQuestEditorView, CView)
	//{{AFX_MSG_MAP(SwgSpaceQuestEditorView)
	ON_WM_SIZE()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_STRINGIDLIST, OnEditStringidlist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

SwgSpaceQuestEditorView::SwgSpaceQuestEditorView() :
	m_initialized(false),
	m_properties(ID_PROPERTYLIST, true),
	m_strings(ID_STRINGLIST, true),
	m_questStrings(ID_QUESTSTRINGLIST, true)
{
	m_properties.setParentInterface(new ParentInterface(this));
	m_strings.setParentInterface(new ParentInterface(this));
	m_questStrings.setParentInterface(new ParentInterface(this));
}

// ----------------------------------------------------------------------

SwgSpaceQuestEditorView::~SwgSpaceQuestEditorView()
{
}

// ----------------------------------------------------------------------

BOOL SwgSpaceQuestEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorView::OnDraw(CDC* /*pDC*/)
{
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void SwgSpaceQuestEditorView::AssertValid() const
{
	CView::AssertValid();
}

void SwgSpaceQuestEditorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorView::addStringId(CString const & stringId)
{
	SwgSpaceQuestEditorDoc const * const document = safe_cast<SwgSpaceQuestEditorDoc const *>(GetDocument());
	CString const & missionTemplateType = document->getMissionTemplateType();
	Configuration::MissionTemplate const * const missionTemplate = Configuration::getMissionTemplate(missionTemplateType);
	Configuration::MissionTemplate::StringTemplate const * stringTemplate = missionTemplate->getStringTemplate(stringId);
	SpaceQuest const & spaceQuest = document->getSpaceQuest();

	CString const & stringData = spaceQuest.getString(stringId);
	CString const stringToolTip = stringTemplate ? stringTemplate->m_toolTipText : "";
	CString const prefix = stringTemplate ? (stringTemplate->m_required ? "*" : "") : "+";

	m_strings.addString(0, prefix + stringId, ID_PROPERTY_TEXT, stringData, stringToolTip, 0, DT_RIGHT);
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorView::removeStringId(CString const & stringId)
{
	m_strings.removeString('+' + stringId);
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	SwgSpaceQuestEditorDoc const * const document = safe_cast<SwgSpaceQuestEditorDoc const *>(GetDocument());
	CString const & missionTemplateType = document->getMissionTemplateType();
	Configuration::MissionTemplate const * const missionTemplate = Configuration::getMissionTemplate(missionTemplateType);
	SpaceQuest const & spaceQuest = document->getSpaceQuest();

	//-- Create the property list
	CRect rect;
	GetClientRect(&rect);
	rect.bottom /= 3;

	int const offset = rect.bottom;
	m_properties.Create(WS_CHILD|WS_BORDER|WS_VISIBLE|LBS_NOTIFY|WS_VSCROLL|WS_HSCROLL|LBS_HASSTRINGS|LBS_OWNERDRAWFIXED, rect, this, ID_PROPERTYLIST);

	{
		//-- Add the property list headers
		m_properties.addString(0, "Property", ID_PROPERTY_STATIC, "Value", "", 0, DT_RIGHT);
		m_properties.addString(0, "Mission Template", ID_PROPERTY_STATIC, missionTemplateType, "", 0, DT_RIGHT);

		//-- Populate the property list
		{
			for (int i = 0; i < missionTemplate->getNumberOfPropertyTemplates(); ++i)
			{
				Configuration::MissionTemplate::PropertyTemplate const & propertyTemplate = missionTemplate->getPropertyTemplate(i);

				CString const & propertyName = propertyTemplate.m_columnName;
				int const propertyType = getPropertyType(propertyTemplate);
				CString const propertyData(getPropertyData(propertyTemplate, spaceQuest));
				int const propertySelection = getPropertySelection(propertyTemplate, spaceQuest);
				CString const & propertyToolTip = propertyTemplate.m_toolTipText;
				CString const prefix = propertyTemplate.m_required ? "*" : "";

				if (!m_properties.addString(&propertyTemplate, prefix + propertyName, propertyType, propertyData, propertyToolTip, propertySelection, DT_RIGHT, FALSE, FALSE))
					FATAL(true, ("AddString failed for property name=%s, type=%i, data=%s, selection=%i\n", propertyName, propertyType, propertyData, propertySelection));
			}
		}

		//-- Force an initial save for default parameters when opening a new document.  If the property has not changed, it won't affect the modified status of the document
		{
			for (int i = 0; i < m_properties.GetCount(); ++i)
				onPropertyChanged(ID_PROPERTYLIST << 16 | i, 0);
		}
	}

	//-- Create the string list
	rect.OffsetRect(0, offset);
	m_strings.Create(WS_CHILD|WS_BORDER|WS_VISIBLE|LBS_NOTIFY|WS_VSCROLL|WS_HSCROLL|LBS_HASSTRINGS|LBS_OWNERDRAWFIXED, rect, this, ID_PROPERTYLIST);

	StringSet stringSet;
	{
		{
			for (int i = 0; i < spaceQuest.getNumberOfStrings(); ++i)
				stringSet.insert(spaceQuest.getStringKey(i));
		}

		{
			for (int i = 0; i < missionTemplate->getNumberOfStringTemplates(); ++i)
				stringSet.insert(missionTemplate->getStringTemplate(i).m_stringId);
		}
	}

	{
		m_strings.addString(0, "StringId", ID_PROPERTY_STATIC, "Text", "", 0, DT_RIGHT);

		for (StringSet::iterator iter = stringSet.begin(); iter != stringSet.end(); ++iter)
			if (!missionTemplate->isQuestStringSuffixTemplate(*iter))
				addStringId(*iter);
	}

	//-- Create the quest string suffix list
	rect.OffsetRect(0, offset);
	m_questStrings.Create(WS_CHILD|WS_BORDER|WS_VISIBLE|LBS_NOTIFY|WS_VSCROLL|WS_HSCROLL|LBS_HASSTRINGS|LBS_OWNERDRAWFIXED, rect, this, ID_QUESTSTRINGLIST);

	{
		//-- Add the property list headers
		m_questStrings.addString(0, "Quest Log Data", ID_PROPERTY_STATIC, "Text", "", 0, DT_RIGHT);
		m_questStrings.addString(0, "CATEGORY", ID_PROPERTY_COMBO_LIST, Configuration::getQuestCategoryList(), "Quest log category", Configuration::getQuestCategoryIndex(spaceQuest.getQuestCategory()), DT_RIGHT, FALSE, FALSE);
		m_questStrings.addString(0, "title_d", ID_PROPERTY_TEXT, spaceQuest.getString("title_d"), "", 0, DT_RIGHT);

		//-- Populate the property list
		{
			for (int i = 0; i < missionTemplate->getNumberOfQuestStringSuffixTemplates(); ++i)
			{
				Configuration::MissionTemplate::QuestStringSuffixTemplate const & questStringSuffixTemplate = missionTemplate->getQuestStringSuffixTemplate(i);
				m_questStrings.addString(0, questStringSuffixTemplate.m_suffix + "_t", ID_PROPERTY_TEXT, spaceQuest.getString(questStringSuffixTemplate.m_suffix + "_t"), questStringSuffixTemplate.m_toolTipText + " title", 0, DT_RIGHT);
				m_questStrings.addString(0, questStringSuffixTemplate.m_suffix + "_d", ID_PROPERTY_TEXT, spaceQuest.getString(questStringSuffixTemplate.m_suffix + "_d"), questStringSuffixTemplate.m_toolTipText + " description", 0, DT_RIGHT);
			}
		}

		//-- Force an initial save for default parameters when opening a new document.  If the property has not changed, it won't affect the modified status of the document
		{
			for (int i = 0; i < m_questStrings.GetCount(); ++i)
				onPropertyChanged(ID_QUESTSTRINGLIST << 16 | i, 0);
		}
	}

	m_initialized = true;
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorView::onPropertyChanged(UINT const hiControlId_loItemChanged, LONG const /*propertyType*/) const
{
	int const controlId = hiControlId_loItemChanged >> 16;
	int const itemChanged = hiControlId_loItemChanged & 0xffff;

	SwgSpaceQuestEditorDoc * const document = safe_cast<SwgSpaceQuestEditorDoc *>(GetDocument());
	CString const & missionTemplateType = document->getMissionTemplateType();
	Configuration::MissionTemplate const * const missionTemplate = Configuration::getMissionTemplate(missionTemplateType);
	SpaceQuest & spaceQuest = document->getSpaceQuest();

	UNREF(itemChanged);

	switch (controlId)
	{
	case ID_PROPERTYLIST:
		{
			CString key;
			m_properties.getPropertyName(itemChanged, key);
			if (!key.IsEmpty() && key[0] == '*')
				key = key.Right(key.GetLength() - 1);

			if (missionTemplate->getPropertyTemplate(key))
			{
				CString value;
				if (!m_properties.getProperty(itemChanged, value))
					FATAL(true, ("getProperty failed for property key=%s, value=%s\n", key, value));

				CString const currentValue = spaceQuest.getProperty(key);
				if (currentValue != value)
				{
					spaceQuest.setProperty(key, value);

					document->SetModifiedFlag(true);
				}
			}
			else
			{
				if (key != "Property" && key != "Mission Template")
					FATAL(true, ("property %s is not present in mission template %s", key, missionTemplate->getName()));
			}
		}
		break;

	case ID_STRINGLIST:
		{
			CString key;
			m_strings.getPropertyName(itemChanged, key);
			if (!key.IsEmpty() && (key[0] == '*' || key[0] == '+'))
				key = key.Right(key.GetLength() - 1);

			CString value;
			m_strings.getProperty(itemChanged, value);

			CString const currentValue = spaceQuest.getString(key);
			if (currentValue != value)
			{
				spaceQuest.setString(key, value);

				document->SetModifiedFlag(true);
			}
		}
		break;

	case ID_QUESTSTRINGLIST:
		{
			CString key;
			m_questStrings.getPropertyName(itemChanged, key);

			if (key != "Quest Log Data")
			{
				if (key == "CATEGORY")
				{
					CString value;
					m_questStrings.getProperty(itemChanged, value);

					CString const currentValue = spaceQuest.getQuestCategory();
					if (currentValue != value)
					{
						spaceQuest.setQuestCategory(value);

						document->SetModifiedFlag(true);
					}
				}
				else
				{
					CString value;
					m_questStrings.getProperty(itemChanged, value);

					CString const currentValue = spaceQuest.getString(key);
					if (currentValue != value)
					{
						spaceQuest.setString(key, value);

						document->SetModifiedFlag(true);
					}
				}
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	if (m_initialized)
	{
		CRect rect;
		GetClientRect(&rect);

		int const numberOfProperties = 1 + m_properties.GetCount();
		int const numberOfStrings = 1 + m_strings.GetCount();
		int const numberOfQuestStrings = 1 + m_questStrings.GetCount();
		int const propertyItemHeight = 16;

		if (numberOfProperties * propertyItemHeight + numberOfStrings * propertyItemHeight + numberOfQuestStrings * propertyItemHeight < rect.bottom)
		{
			rect.bottom = numberOfProperties * propertyItemHeight;
			m_properties.MoveWindow(rect);

			rect.top = 0;
			rect.bottom = numberOfStrings * propertyItemHeight;
			rect.OffsetRect(0, numberOfProperties * propertyItemHeight);
			m_strings.MoveWindow(rect);

			rect.top = 0;
			rect.bottom = numberOfQuestStrings * propertyItemHeight;
			rect.OffsetRect(0, numberOfProperties * propertyItemHeight + numberOfStrings * propertyItemHeight);
			m_questStrings.MoveWindow(rect);
		}
		else
		{
			int const offset = rect.bottom /= 3;
			rect.bottom = offset;
			m_properties.MoveWindow(rect);

			rect.OffsetRect(0, offset);
			m_strings.MoveWindow(rect);

			rect.OffsetRect(0, offset);
			m_questStrings.MoveWindow(rect);
		}
	}
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorView::OnEditCopy() 
{
	if (dynamic_cast<CEdit *>(GetActiveWindow()->GetFocus()))
		safe_cast<CEdit *>(GetActiveWindow()->GetFocus())->Copy();
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorView::OnEditCut() 
{
	if (dynamic_cast<CEdit *>(GetActiveWindow()->GetFocus()))
		safe_cast<CEdit *>(GetActiveWindow()->GetFocus())->Cut();
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorView::OnEditPaste() 
{
	if (dynamic_cast<CEdit *>(GetActiveWindow()->GetFocus()))
		safe_cast<CEdit *>(GetActiveWindow()->GetFocus())->Paste();
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorView::OnEditUndo() 
{
	if (dynamic_cast<CEdit *>(GetActiveWindow()->GetFocus()))
		safe_cast<CEdit *>(GetActiveWindow()->GetFocus())->Undo();
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorView::OnEditStringidlist() 
{
	//-- Create list of ids that aren't in mission templates
	StringList stringList;
	for (int i = 0; i < m_strings.GetCount(); ++i)
	{
		CString propertyName;
		m_strings.getPropertyName(i, propertyName);

		if (!propertyName.IsEmpty() && propertyName[0] == '+')
			stringList.push_back(propertyName.Right(propertyName.GetLength() - 1));
	}

	if (!stringList.empty())
	{
		SwgSpaceQuestEditorDoc * const document = safe_cast<SwgSpaceQuestEditorDoc *>(GetDocument());
		CString const & missionTemplateType = document->getMissionTemplateType();
		SpaceQuest & spaceQuest = document->getSpaceQuest();

		DialogStringIdList dlg(missionTemplateType, stringList);
		if (dlg.DoModal() == IDOK)
		{
			StringList newStringList = dlg.getStringIdList();

			//-- Any strings in the new list that aren't in the old list are added
			{
				for (StringList::iterator iter = newStringList.begin(); iter != newStringList.end(); ++iter)
				{
					if (std::find(stringList.begin(), stringList.end(), *iter) == stringList.end())
					{
						//-- Add string to space quest
						spaceQuest.setString(*iter, "");

						//-- Add string to m_strings
						addStringId(*iter);
					}
				}
			}

			//-- Any strings in the old list that aren't in the new list are removed
			{
				for (StringList::iterator iter = stringList.begin(); iter != stringList.end(); ++iter)
				{
					if (std::find(newStringList.begin(), newStringList.end(), *iter) == newStringList.end())
					{
						//-- Remove string from space quest
						spaceQuest.removeString(*iter);

						//-- Remove string from m_strings
						removeStringId(*iter);
					}
				}
			}			
		}
	}
	else
		MessageBox("There are no user-defined strings to edit.");
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorView::OnUpdate(CView * const pSender, LPARAM const lHint, CObject * const /*pHint*/) 
{
	if (pSender != this)
	{
		if (lHint == SwgSpaceQuestEditorDoc::H_aboutToSave)
			SetFocus();
	}
}

// ----------------------------------------------------------------------

SwgSpaceQuestEditorView::ParentInterface::ParentInterface(SwgSpaceQuestEditorView const * const editorView) :
	m_editorView(editorView)
{
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorView::ParentInterface::onPropertyChanged(UINT const hiControlId_loItemChanged, LONG const lPropertyType) const
{
	m_editorView->onPropertyChanged(hiControlId_loItemChanged, lPropertyType);
}

// ======================================================================

