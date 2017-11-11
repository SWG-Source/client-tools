// ======================================================================
//
// SpaceZonePropertyView.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceZoneEditor.h"
#include "SpaceZonePropertyView.h"

#include "Configuration.h"
#include "PropertyListCtrl.h"
#include "Resource.h"
#include "SwgSpaceZoneEditorDoc.h"

// ======================================================================

namespace SpaceZonePropertyViewNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - =

	int getPropertyType(Configuration::PropertyTemplate const & propertyTemplate)
	{
		switch (propertyTemplate.m_propertyType)
		{
		case Configuration::PropertyTemplate::PT_bool:
			return ID_PROPERTY_BOOL;
		
		case Configuration::PropertyTemplate::PT_float:
		case Configuration::PropertyTemplate::PT_integer:
			return ID_PROPERTY_INTEGER;

		case Configuration::PropertyTemplate::PT_string:
			return ID_PROPERTY_TEXT;

		case Configuration::PropertyTemplate::PT_enumList:
			return ID_PROPERTY_COMBO_LIST;

		case Configuration::PropertyTemplate::PT_spaceMobile:
			return ID_PROPERTY_TEXT_SPACEMOBILE;

		case Configuration::PropertyTemplate::PT_spaceMobileList:
			return ID_PROPERTY_TEXT_SPACEMOBILELIST;
		}

		return ID_PROPERTY_STATIC;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - =

	CString const getPropertyData(Configuration::PropertyTemplate const & propertyTemplate, SwgSpaceZoneEditorDoc::Object const & object)
	{
		switch (propertyTemplate.m_propertyType)
		{
		case Configuration::PropertyTemplate::PT_spaceMobile:
			return object.getObjVar(propertyTemplate.m_name);

		case Configuration::PropertyTemplate::PT_spaceMobileList:
			return object.getObjVar(propertyTemplate.m_name);

		case Configuration::PropertyTemplate::PT_bool:
			return "false!true";

		case Configuration::PropertyTemplate::PT_float:
		case Configuration::PropertyTemplate::PT_integer:
			return object.getObjVar(propertyTemplate.m_name);

		case Configuration::PropertyTemplate::PT_string:
			return object.getObjVar(propertyTemplate.m_name);

		case Configuration::PropertyTemplate::PT_enumList:
			{
				CString result;
				Configuration::packString(propertyTemplate.m_propertyDataList, result, '!');
				return result;
			}
		}

		return "ERROR(2): see asommers";
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - =

	int getPropertySelection(Configuration::PropertyTemplate const & propertyTemplate, SwgSpaceZoneEditorDoc::Object const & object)
	{
		switch (propertyTemplate.m_propertyType)
		{
		case Configuration::PropertyTemplate::PT_bool:
			{
				CString const property = object.getObjVar(propertyTemplate.m_name);
				if (property.IsEmpty())
					return 0;

				return property[0] == '0' ? 0 : 1;
			}

		case Configuration::PropertyTemplate::PT_float:
		case Configuration::PropertyTemplate::PT_integer:
			return 0;

		case Configuration::PropertyTemplate::PT_string:
			return 0;

		case Configuration::PropertyTemplate::PT_enumList:
			{
				StringList::const_iterator iter = std::find(propertyTemplate.m_propertyDataList.begin(), propertyTemplate.m_propertyDataList.end(), object.getObjVar(propertyTemplate.m_name));
				if (iter == propertyTemplate.m_propertyDataList.end())
					return 0;

				return iter - propertyTemplate.m_propertyDataList.begin();
			}

		case Configuration::PropertyTemplate::PT_spaceMobile:
			return 0;

		case Configuration::PropertyTemplate::PT_spaceMobileList:
			return 0;
		}

		return 0;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - =
}

using namespace SpaceZonePropertyViewNamespace;

// ======================================================================

IMPLEMENT_DYNCREATE(SpaceZonePropertyView, CView)

BEGIN_MESSAGE_MAP(SpaceZonePropertyView, CView)
	//{{AFX_MSG_MAP(SpaceZonePropertyView)
	ON_WM_SIZE()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	//ON_LBN_SELCHANGE(ID_PROPERTYLIST, OnPropertyChanged)
	//}}AFX_MSG_MAP
	ON_MESSAGE(ID_PROPERTY_CHANGED, OnPropertyChanged)
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

SpaceZonePropertyView::SpaceZonePropertyView() :
	m_initialized(false),
	m_properties(0),
	m_object(0)
{
}

// ----------------------------------------------------------------------

SpaceZonePropertyView::~SpaceZonePropertyView()
{
	delete m_properties;
}

// ----------------------------------------------------------------------

BOOL SpaceZonePropertyView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

// ----------------------------------------------------------------------

void SpaceZonePropertyView::OnDraw(CDC* /*pDC*/)
{
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void SpaceZonePropertyView::AssertValid() const
{
	CView::AssertValid();
}

void SpaceZonePropertyView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void SpaceZonePropertyView::createPropertyListCtrl() 
{
	CRect rect;
	GetClientRect(&rect);
	delete m_properties;
	m_properties = new PropertyListCtrl(ID_PROPERTYLIST, true);
	m_properties->Create(WS_CHILD|WS_BORDER|WS_VISIBLE|LBS_NOTIFY|WS_VSCROLL|WS_HSCROLL|LBS_HASSTRINGS|LBS_OWNERDRAWFIXED, rect, this, ID_PROPERTYLIST);
}

// ----------------------------------------------------------------------

void SpaceZonePropertyView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	//-- Create the property list
	createPropertyListCtrl();

	m_initialized = true;
}

// ----------------------------------------------------------------------

LONG SpaceZonePropertyView::OnPropertyChanged(UINT const hiControlId_loItemChanged, LONG const /*propertyType*/)
{
	if (m_object)
	{
		int const controlId = hiControlId_loItemChanged >> 16;
		int const itemChanged = hiControlId_loItemChanged & 0xffff;

		if (controlId == ID_PROPERTYLIST)
		{
			CString key;
			m_properties->getPropertyName(itemChanged, key);

			if (key != "Name")
			{
				CString value;
				if (!m_properties->getProperty(itemChanged, value))
					FATAL(true, ("getProperty failed for property key=%s, value=%s\n", key, value));

				CString const currentValue = m_object->getObjVar(key);
				if (currentValue != value)
				{
					m_object->setObjVar(key, value);
					GetDocument()->SetModifiedFlag(true);
				}
			}
		}
	}

	return 1;
}

// ----------------------------------------------------------------------

void SpaceZonePropertyView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	if (m_initialized)
	{
		CRect rect;
		GetClientRect(&rect);
		m_properties->MoveWindow(rect);
	}
}

// ----------------------------------------------------------------------

void SpaceZonePropertyView::OnEditCopy() 
{
	if (dynamic_cast<CEdit *>(GetActiveWindow()->GetFocus()))
		safe_cast<CEdit *>(GetActiveWindow()->GetFocus())->Copy();
}

// ----------------------------------------------------------------------

void SpaceZonePropertyView::OnEditCut() 
{
	if (dynamic_cast<CEdit *>(GetActiveWindow()->GetFocus()))
		safe_cast<CEdit *>(GetActiveWindow()->GetFocus())->Cut();
}

// ----------------------------------------------------------------------

void SpaceZonePropertyView::OnEditPaste() 
{
	if (dynamic_cast<CEdit *>(GetActiveWindow()->GetFocus()))
		safe_cast<CEdit *>(GetActiveWindow()->GetFocus())->Paste();
}

// ----------------------------------------------------------------------

void SpaceZonePropertyView::OnEditUndo() 
{
	if (dynamic_cast<CEdit *>(GetActiveWindow()->GetFocus()))
		safe_cast<CEdit *>(GetActiveWindow()->GetFocus())->Undo();
}

// ----------------------------------------------------------------------

void SpaceZonePropertyView::OnUpdate(CView * const pSender, LPARAM const lHint, CObject * const pHint) 
{
	//-- This is overloaded so the PropertyView won't continue to 
	//   redraw when the mouse moves in the MapView
	if (pSender != this)
	{
		if (lHint == SwgSpaceZoneEditorDoc::H_selectObject)
		{
			NOT_NULL(m_properties);

			//-- Make sure we have focus to stop any edits
			SetFocus();

			//-- Save all existing properties
			{
				for (int i = 0; i < m_properties->GetCount(); ++i)
					OnPropertyChanged(ID_PROPERTYLIST << 16 | i, 0);
			}

			//-- Recreate the property list ctrl
			createPropertyListCtrl();

			//-- Populate with items
			SwgSpaceZoneEditorDoc::Object * const object = reinterpret_cast<SwgSpaceZoneEditorDoc::Object *>(pHint);
			m_object = object;
			if (object)
			{
				Configuration::ObjVarList const & objVarList = object->getObjVarList();

				{
					m_properties->addString("Name", ID_PROPERTY_STATIC, "Value", "", 0, DT_RIGHT);

					//-- Populate the property list
					for (size_t i = 0; i < objVarList.size(); ++i)
					{
						Configuration::ObjVar const & objVar = objVarList[i];

						CString const & propertyName = objVar.m_key;
						Configuration::PropertyTemplate const & propertyTemplate = *NON_NULL(Configuration::getPropertyTemplate(propertyName));
						if (!propertyTemplate.m_hidden)
						{
							int const propertyType = getPropertyType(propertyTemplate);
							CString const propertyData(getPropertyData(propertyTemplate, *object));
							int const propertySelection = getPropertySelection(propertyTemplate, *object);
							CString const & propertyToolTip = propertyTemplate.m_toolTipText;

							if (!m_properties->addString(propertyName, propertyType, propertyData, propertyToolTip, propertySelection, DT_RIGHT, FALSE, FALSE))
								FATAL(true, ("AddString failed for property name=%s, type=%i, data=%s, selection=%i\n", propertyName, propertyType, propertyData, propertySelection));
						}
					}

					//-- Force an initial save for default parameters when opening a new document.  If the property has not changed, it won't affect the modified status of the document
					{
						for (int i = 0; i < m_properties->GetCount(); ++i)
							OnPropertyChanged(ID_PROPERTYLIST << 16 | i, 0);
					}
				}
			}
		}
	}
}

// ======================================================================

