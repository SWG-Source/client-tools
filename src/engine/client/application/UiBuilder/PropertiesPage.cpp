// PropertiesPage.cpp : implementation file
//

#include "FirstUiBuilder.h"
#include "PropertiesPage.h"
#include "EditUtils.h"
#include "ObjectEditor.h"
#include "EditorMonitor.h"
#include "PropertyListControl.h"
#include "UIBaseObject.h"

#include <algorithm>

namespace PropertiesPageNamespace {

	// ================================================================

	typedef UIBaseObject::UIPropertyGroupVector  UIPropertyGroupVector;

	typedef UIBaseObject::UIObjectVector         UIObjectVector;

	// ================================================================

	class PropertyList;

	class Property : public PropertyListControl::Property
	{
	public:

		typedef PropertyListControl::Property baseclass;

		// ---------------------------------------

		Property(
			PropertyListControl        &ownerControl, 
			const UIPropertyDescriptor &i_descriptor, 
			PropertyList               &ownerList
		)
		:	PropertyListControlProperty(ownerControl, i_descriptor),
			m_ownerList(ownerList)
		{}

		// ---------------------------------------

		ObjectEditor         &getEditor();
		const UIObjectVector &getObjects() const;
		UIBaseObject         *getAnchor();

		// ---------------------------------------

		virtual bool setValue(const CString &i_newValue);
		virtual void onFollowLink();

		void updateValue(const UIBaseObject &anchorSelection);

		// ---------------------------------------

		PropertyList                &m_ownerList;
	};

	// ================================================================


	// ================================================================

	class PropertyList : public ObjectPropertiesEditor::PropertyList
	{
	public:

		enum { PAD = 4 };

		PropertyList(const UIPropertyGroup &i_propertyGroup, PropertiesPage &owner, int topY);
		~PropertyList();

		PropertiesPage &getOwner() { return *static_cast<PropertiesPage *>(&m_owner); }

		// -----------------------------------------------

		Property   *getProperty(int i) const { return m_dialogProperties[i]; }

		int getTopY()    const { return m_topY; }
		int getBottomY() const { return m_bottomY; }
		int getHeight()  const { return getBottomY() - getTopY(); }

		// -----------------------------------------------

		void createControls(int topY);

		void setTopY(int newTopY);

		// -----------------------------------------------
		virtual void clear();
		virtual void onPropertyGroupChanged();
		virtual void addObject(UIBaseObject &o);
		virtual bool removeObject(UIBaseObject &o);
		virtual void onSetValue(UIBaseObject &o, const char *i_propertyName);
		// -----------------------------------------------

		UIBaseObject *getAnchor() { if (empty()) return 0; else return _getAnchor(); }
		UIBaseObject *_getAnchor() { return m_objects.back(); }

		void _initializeListControl(const CRect *listRect=0);
		void _resizeListControl(const CRect *listRect=0);
		void _updateValues(UIBaseObject &o);
		void _retextLabel();

		// -----------------------------------------------

		Property              **m_dialogProperties;

		CButton                *m_label;
		PropertyListControl    *m_listControl;
		int                     m_topY;
		int                     m_listY;
		int                     m_bottomY;
	};

	PropertyList::PropertyList(const UIPropertyGroup &i_propertyGroup, PropertiesPage &owner, int topY)
	:	ObjectPropertiesEditor::PropertyList(i_propertyGroup, owner),
		m_dialogProperties(0),
		m_label(0),
		m_listControl(0),
		m_topY(topY),
		m_listY(0),
		m_bottomY(0)
	{
		if (owner.m_hWnd)
		{
			createControls(topY);
		}
	}

	// -------------------------------------------------------------------

	PropertyList::~PropertyList()
	{
		delete m_label;
		delete m_listControl;
		delete [] m_dialogProperties;
	}

	// -------------------------------------------------------------------

	void PropertyList::createControls(int topY)
	{
		// --------------------------
		// set new topY value
		if (m_listControl)
		{
			if (topY != getTopY())
			{
				setTopY(topY);
			}
			return;
		}
		m_topY=topY;
		// --------------------------

		const int fontHeight = 20;

		CRect pageRect;
		getOwner().GetClientRect(&pageRect);

		// -------------------------------------------------------------------
		// Create group label.
		CRect staticRect;
		staticRect.left = pageRect.left;
		staticRect.right = pageRect.right;
		staticRect.top = m_topY;
		staticRect.bottom = m_topY + fontHeight;
		m_label = new CButton();
		m_label->Create(getGroupName(), BS_LEFTTEXT | WS_CHILD | WS_VISIBLE, staticRect, &getOwner(), -1);
		_retextLabel();
		// -------------------------------------------------------------------

		// -------------------------------------------------------------------
		// Create property list control
		m_listY = staticRect.bottom;

		CRect listRect;
		listRect.left = pageRect.left;
		listRect.right = pageRect.right;
		listRect.top = m_listY;
		listRect.bottom = listRect.top + getPropertyCount()*fontHeight;

		m_listControl = new PropertyListControl();
		m_listControl->Create(
			LVS_REPORT | LVS_SINGLESEL | LVS_OWNERDRAWFIXED | WS_BORDER | LVS_NOCOLUMNHEADER, 
			listRect, 
			&getOwner(), 
			-1
		);
		// -------------------------------------------------------------------

		// -------------------------------------------------------------------
		// Initialize and show the list control
		_initializeListControl(&listRect);

		m_listControl->ShowWindow(SW_SHOW);
		// -------------------------------------------------------------------

		// -------------------------------------------------------------------
		if (!m_objects.empty())
		{
			_updateValues(*_getAnchor());
		}
	}

	// -------------------------------------------------------------------

	void PropertyList::setTopY(int newTopY)
	{
		int diff = newTopY - m_topY;
		if (diff==0)
		{
			return;
		}

		m_topY = newTopY;

		// move all controls down by "diff" units.
		CRect rect;
		if (m_label)
		{
			m_label->GetClientRect(&rect);
			rect.top = m_topY;
			rect.bottom += rect.top;
			m_label->MoveWindow(&rect);
		}

		if (m_listControl)
		{
			m_listY+=diff;
			m_listControl->GetClientRect(&rect);
			rect.top = m_listY;
			rect.bottom += rect.top;
			m_listControl->MoveWindow(&rect);
		}

		m_bottomY += diff;
	}

	// -------------------------------------------------------------------------------

	void PropertyList::clear()
	{
		m_objects.clear();
		if (m_dialogProperties)
		{
			delete [] m_dialogProperties;
			m_dialogProperties=0;
		}
		if (m_listControl)
		{
			m_listControl->clear();
			_resizeListControl();
		}
	}

	// -------------------------------------------------------------------------------

	void PropertyList::onPropertyGroupChanged()
	{
		if (m_listControl)
		{
			m_listControl->clear();
		}
		if (m_dialogProperties)
		{
			delete [] m_dialogProperties;
			m_dialogProperties=0;
		}

		_initializeListControl();
		// TODO - notify parent to re-pack dialog??
	}

	// -------------------------------------------------------------------------------

	void PropertyList::addObject(UIBaseObject &o)
	{
		m_objects.push_back(&o);
		_updateValues(o);
		_retextLabel();
	}

	// -------------------------------------------------------------------------------

	bool PropertyList::removeObject(UIBaseObject &o)
	{
		std::vector<UIBaseObject *>::iterator oi = std::find(m_objects.begin(), m_objects.end(), &o);
		if (oi==m_objects.end())
		{
			return false;
		}

		const bool wasAnchor = (oi==m_objects.begin());

		m_objects.erase(oi);

		if (wasAnchor && !m_objects.empty())
		{
			_updateValues(*_getAnchor());
		}

		return true;
	}

	// -------------------------------------------------------------------------------

	void PropertyList::onSetValue(UIBaseObject &o, const char *i_propertyName)
	{
		if (!m_objects.empty() && &o==_getAnchor())
		{
			_updateValues(o);
		}
	}

	// -------------------------------------------------------------------------------

	void PropertyList::_initializeListControl(const CRect *i_listRect)
	{
		assert(!m_dialogProperties);
		if (!m_listControl)
		{
			return;
		}
		assert(m_listControl->empty());

		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

		const UIPropertyDescriptor *descs = m_propertyGroup.fields;
		const int fieldCount = getPropertyCount();

		m_dialogProperties = new Property *[fieldCount];
		for (int i=0;i<fieldCount;i++)
		{
			const UIPropertyDescriptor &pd = descs[i];
			Property *p = new Property(*m_listControl, pd, *this);
			m_dialogProperties[i] = p;
			m_listControl->addProperty(*p);
		}

		// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

		_resizeListControl(i_listRect);
	}

	// -------------------------------------------------------------------------------

	void PropertyList::_resizeListControl(const CRect *i_listRect)
	{
		CRect listRect;
		if (i_listRect)
		{
			listRect=*i_listRect;
		}
		else
		{
			getOwner().GetClientRect(&listRect);
		}
		const CSize listSize = m_listControl->ApproximateViewRect(CSize(listRect.Width(), -1), getPropertyCount());
		listRect.top = m_listY;
		listRect.bottom = listRect.top + listSize.cy;
		m_listControl->MoveWindow(listRect);
		m_bottomY = listRect.bottom + PAD;
	}

	// -------------------------------------------------------------------------------

	void PropertyList::_updateValues(UIBaseObject &o)
	{
		if (!m_listControl)
		{
			return;
		}
		for (int i=0;i<getPropertyCount();i++)
		{
			Property *p = m_dialogProperties[i];
			p->updateValue(o);
		}
	}

	// -------------------------------------------------------------------------------

	void PropertyList::_retextLabel()
	{
		if (!m_label)
		{
			return;
		}
		char labelText[256];
		sprintf(labelText, "%s (%i)", getGroupName(), m_objects.size());
		m_label->SetWindowText(labelText);
	}

	// ================================================================

	inline ObjectEditor         &Property::getEditor()        { return m_ownerList.getOwner().getEditor(); }
	inline const UIObjectVector &Property::getObjects() const { return m_ownerList.getObjects(); }
	inline UIBaseObject         *Property::getAnchor()        { return m_ownerList.getAnchor(); }

	// -------------------------------------------------------------------------------

	void Property::updateValue(const UIBaseObject &anchorSelection)
	{
		UILowerString name(m_descriptor.m_name); // TODO OPTIMIZE
		anchorSelection.GetPropertyNarrow(name, m_narrowValue);
		baseclass::refreshItemValue();
	}

	// -------------------------------------------------------------------------------

	bool Property::setValue(const CString &i_newValue)
	{
		// Do not call base-class implementation.
		// Send a request to the editor class
		UILowerString name(m_descriptor.m_name);
		UINarrowString nValue(i_newValue);

		UIString value(UIUnicode::narrowToWide(nValue));

		return getEditor().setObjectProperty(getObjects(), name, value);
	}

	// -------------------------------------------------------------------------------

	void Property::onFollowLink()
	{
		UIBaseObject *o = getAnchor();
		if (o)
		{
			UIBaseObject *const linkedObject = o->GetObjectFromPath(m_narrowValue.c_str());

			if (linkedObject)
			{
				// NOTE: is is possible that this object will get delected inside
				// the selection call.
				getEditor().select(*linkedObject, ObjectEditor::SEL_ASSIGN);
			}
		}
	}

	// -------------------------------------------------------------------------------

}
using namespace PropertiesPageNamespace;

/////////////////////////////////////////////////////////////////////////////
// PropertiesPage property page
PropertiesPage::PropertiesPage(ObjectEditor &i_editor, UIPropertyCategories::Category i_category)
:	CPropertyPage(PropertiesPage::IDD),
	ObjectPropertiesEditor::PropertyCategory(i_editor, i_category)
{
	m_psp.dwFlags |= PSP_USETITLE;

	char tabText[256];
	getLabelText(tabText, sizeof(tabText));
	m_strCaption=tabText;
	m_psp.pszTitle = m_strCaption;
	//{{AFX_DATA_INIT(PropertiesPage)
	//}}AFX_DATA_INIT
}

PropertiesPage::~PropertiesPage()
{
}

/////////////////////////////////////////////////////////////////////////////

// ==========================================================================

ObjectPropertiesEditor::PropertyList *PropertiesPage::_newPropertyList(
	const UIPropertyGroup &propertyGroup, 
	const PropertyListMap::iterator &insertionPoint
)
{
	int topY=0;
	if (insertionPoint!=m_propertyLists.begin())
	{
		PropertyListMap::iterator prev = insertionPoint;
		--prev;
		PropertyList *pl = static_cast<PropertyList *>(prev->second);
		topY = pl->getBottomY();
	}

	// ----------------------------------------------------------------

	PropertyList *pl = new PropertyList(propertyGroup, *this, topY);
	std::pair<const PropertyListMap::key_type, PropertyList *> value(&propertyGroup, pl);
	m_propertyLists.insert(insertionPoint, value);

	// ----------------------------------------------------------------

	{
		int newY = pl->getBottomY();
		for (PropertyListMap::iterator shiftIter = insertionPoint; shiftIter!=m_propertyLists.end(); ++shiftIter)
		{
			PropertyList *shiftProp = static_cast<PropertyList *>(shiftIter->second);
			shiftProp->setTopY(newY);
			newY = shiftProp->getBottomY();
		}
	}

	return pl;
}

// ==========================================================================

void PropertiesPage::_freePropertyList(
	ObjectPropertiesEditor::PropertyList *i_pl,
	const PropertyListMap::iterator &listIter
)
{
	PropertyList *pl = static_cast<PropertyList *>(i_pl);

	int topY = pl->getTopY();

	delete pl;

	PropertyListMap::iterator shiftIter = listIter;
	++shiftIter;
	m_propertyLists.erase(listIter);
	while (shiftIter!=m_propertyLists.end())
	{
		PropertyList *const shiftProperty = static_cast<PropertyList *>(shiftIter->second);
		shiftProperty->setTopY(topY);
		++shiftIter;
	}
}

/////////////////////////////////////////////////////////////////////////////

void PropertiesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PropertiesPage)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(PropertiesPage, CPropertyPage)
	//{{AFX_MSG_MAP(PropertiesPage)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PropertiesPage message handlers

BOOL PropertiesPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	char tabText[256];
	getLabelText(tabText, sizeof(tabText));
	SetWindowText(tabText);

	int topY=0;
	for (PropertyListMap::iterator pli=m_propertyLists.begin();pli!=m_propertyLists.end();++pli)
	{
		PropertyList *pl = static_cast<PropertyList *>(pli->second);
		pl->createControls(topY);
		topY = pl->getBottomY();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void PropertiesPage::OnSize(UINT nType, int cx, int cy) 
{
	CPropertyPage::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}
