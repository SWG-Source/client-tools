// ObjectPropertiesTreeDialog.cpp : implementation file
//

#include "FirstUiBuilder.h"
#include "ObjectPropertiesTreeDialog.h"
#include "ObjectEditor.h"
#include "UIBuilder.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

ObjectPropertiesTreeDialog::PropertyList::PropertyList(const UIPropertyGroup &i_propertyGroup, ObjectPropertiesEditor::PropertyCategory &owner)
: ObjectPropertiesEditor::PropertyList(i_propertyGroup, owner)
{
}

// ==========================================================================

ObjectPropertiesTreeDialog::PropertyList::~PropertyList()
{
}

// ==========================================================================

void ObjectPropertiesTreeDialog::PropertyList::addObject(UIBaseObject &o)
{
	m_objects.push_back(&o);
	//_updateValues(o);
	//_retextLabel();
}

// ==========================================================================

bool ObjectPropertiesTreeDialog::PropertyList::removeObject(UIBaseObject &o)
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
		//_updateValues(*_getAnchor());
	}

	return true;
}

// ==========================================================================

void ObjectPropertiesTreeDialog::PropertyList::onSetValue(UIBaseObject &o, const char *i_propertyName)
{
/*
	if (!m_objects.empty() && &o==_getAnchor())
	{
		_updateValues(o);
	}
*/
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

ObjectPropertiesTreeDialog::PropertyCategory::PropertyCategory(ObjectEditor &i_editor, UIPropertyCategories::Category i_category, const TreeItem &i_treeParent)
:	  ObjectPropertiesEditor::PropertyCategory(i_editor, i_category)
	, m_treeParent(i_treeParent)
	, m_treeItem(0)
{
	char labelText[1024];
	getLabelText(labelText, sizeof(labelText));
   m_treeItem=tree().InsertItem(labelText, parentTreeItem(), TVI_LAST);
}

// ==========================================================================

ObjectPropertiesTreeDialog::PropertyCategory::~PropertyCategory()
{
	if (m_treeItem)
	{
		tree().DeleteItem(m_treeItem);
	}
}

// ==========================================================================

ObjectPropertiesEditor::PropertyList *ObjectPropertiesTreeDialog::PropertyCategory::_newPropertyList(
	const UIPropertyGroup &propertyGroup, 
	const PropertyListMap::iterator &insertionPoint
)
{
	PropertyList *pl = new PropertyList(propertyGroup, *this);
	std::pair<const PropertyListMap::key_type, PropertyList *> value(&propertyGroup, pl);
	m_propertyLists.insert(insertionPoint, value);
	return pl;
}

// ==========================================================================

void ObjectPropertiesTreeDialog::PropertyCategory::_freePropertyList(
	ObjectPropertiesEditor::PropertyList *pl,
	const PropertyListMap::iterator &listIter
)
{
	delete pl;
	m_propertyLists.erase(listIter);
}

/////////////////////////////////////////////////////////////////////////////
// ObjectPropertiesTreeDialog dialog
/////////////////////////////////////////////////////////////////////////////

extern CUiBuilderApp theApp;

ObjectPropertiesTreeDialog::ObjectPropertiesTreeDialog(ObjectEditor &i_editor, CWnd* pParent /*=NULL*/)
:	  CDialog(ObjectPropertiesTreeDialog::IDD, pParent)
	, ObjectPropertiesEditor(i_editor)
	, accelerators(0)
	, showActive(false)
{
	//{{AFX_DATA_INIT(ObjectPropertiesTreeDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

   Create(IDD, pParent);

	if (pParent)
	{
		SetParent(pParent);
	}
}

// ==========================================================================

void ObjectPropertiesTreeDialog::destroy()
{
	if (m_hWnd)
	{
		DestroyWindow();
	}
	else
	{
		PostNcDestroy();
	}
}

// ==========================================================================

void ObjectPropertiesTreeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ObjectPropertiesTreeDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

// ==========================================================================

void ObjectPropertiesTreeDialog::setAcceleratorTable(HACCEL x)
{
	accelerators=x;
}

// ==========================================================================

void ObjectPropertiesTreeDialog::saveUserPreferences()
{
	if (IsWindowVisible())
	{
		CRect windowRect;
		GetWindowRect(windowRect);
		theApp.saveDialogPosition("ObjectPropertiesDialog", windowRect);
	}
}

// ==========================================================================

void ObjectPropertiesTreeDialog::setActiveAppearance(bool i_showActive)
{
	if (i_showActive!=showActive)
	{
		showActive=i_showActive;
		PostMessage(WM_NCACTIVATE, showActive);
	}
}

// ==========================================================================

ObjectPropertiesEditor::PropertyCategory *ObjectPropertiesTreeDialog::_createCategory(int categoryIndex)
{
	//const HTREEITEM root = m_propertyTree.GetRootItem(); assert(root);
	PropertyCategory *returnValue = new PropertyCategory(m_editor, UIPropertyCategories::Category(categoryIndex), TreeItem(&m_propertyTree, TVI_ROOT));
	//AddPage(returnValue);
	return returnValue;
}

// ==========================================================================

void ObjectPropertiesTreeDialog::_lock()
{
	m_propertyTree.SetRedraw(false);
}

// ==========================================================================

void ObjectPropertiesTreeDialog::_unlock()
{
	_retextDialog();
	m_propertyTree.SetRedraw(true);
}

// =====================================================================

void ObjectPropertiesTreeDialog::onEditReset()
{
	ObjectPropertiesEditor::onEditReset();
	_retextAllTabs();
	_retextDialog();
}

// =====================================================================

void ObjectPropertiesTreeDialog::onEditInsertSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling)
{
	ObjectPropertiesEditor::onEditInsertSubtree(subTree, previousSibling);
	_retextAllTabs();
	_retextDialog();
}

// =====================================================================

void ObjectPropertiesTreeDialog::onEditRemoveSubtree(UIBaseObject &subTree)
{
	ObjectPropertiesEditor::onEditRemoveSubtree(subTree);
	_retextAllTabs();
	_retextDialog();
}

// =====================================================================

void ObjectPropertiesTreeDialog::onEditMoveSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling, UIBaseObject *oldParent)
{
	ObjectPropertiesEditor::onEditMoveSubtree(subTree, previousSibling, oldParent);
	_retextAllTabs();
	_retextDialog();
}

// =====================================================================

void ObjectPropertiesTreeDialog::onEditSetObjectProperty(UIBaseObject &object, const char *i_propertyName)
{
	ObjectPropertiesEditor::onEditSetObjectProperty(object, i_propertyName);
}

// =====================================================================

void ObjectPropertiesTreeDialog::onSelect(UIBaseObject &object, bool isSelected)
{
	ObjectPropertiesEditor::onSelect(object, isSelected);
	_retextAllTabs();
	_retextDialog();
}

// =====================================================================

void ObjectPropertiesTreeDialog::_retextTab(int category)
{
	const char *categoryName = UIPropertyCategories::s_propertyCategoryNames[category].c_str();
	PropertyCategory *pp = static_cast<PropertyCategory *>(m_propertyCategories[category]);
	char tabText[1024];

	if (pp)
	{
		pp->getLabelText(tabText, sizeof(tabText));
		sprintf(tabText, "%s (%i)", categoryName, pp->getSelectionCount());
	}
	else
	{
		strcpy(tabText, categoryName);
	}

	m_propertyTree.SetItemText(pp->treeItem(), tabText);
}

// =====================================================================

void ObjectPropertiesTreeDialog::_retextAllTabs()
{
	if (!m_hWnd)
	{
		return;
	}
	for (int i=0;i<UIPropertyCategories::C_NUM_CATEGORIES;i++)
	{
		_retextTab(i);
	}
}

// =====================================================================

void ObjectPropertiesTreeDialog::_retextDialog()
{
	if (!m_hWnd || isLocked())
	{
		return;
	}

	CString windowText;
	GetWindowText(windowText);

	int left = windowText.ReverseFind('(');
	if (left==-1)
	{
		left=windowText.GetLength();
	}
	else
	{
		left--;
		windowText.Delete(left, windowText.GetLength() - left);
	}

	char newWindowText[256];
	sprintf(newWindowText, "%s (%i)", static_cast<LPCSTR>(windowText), m_selectionCount);
	SetWindowText(newWindowText);
}

// ==========================================================================

BEGIN_MESSAGE_MAP(ObjectPropertiesTreeDialog, CDialog)
	//{{AFX_MSG_MAP(ObjectPropertiesTreeDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ObjectPropertiesTreeDialog message handlers
/////////////////////////////////////////////////////////////////////////////

void ObjectPropertiesTreeDialog::OnCancel() 
{ 
	if (m_propertyTree.GetEditControl())
	{
		TreeView_EndEditLabelNow(m_propertyTree, TRUE);
	}
}

void ObjectPropertiesTreeDialog::OnOK()
{
	if (m_propertyTree.GetEditControl())
	{
		TreeView_EndEditLabelNow(m_propertyTree, FALSE);
	}
}


void ObjectPropertiesTreeDialog::OnDestroy() 
{
	CDialog::OnDestroy();
}

void ObjectPropertiesTreeDialog::PostNcDestroy() 
{
	m_editor.removeMonitor(*this);
	delete this;
}

BOOL ObjectPropertiesTreeDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

   // setup icon resources
   HINSTANCE hInst=AfxGetInstanceHandle();

	//iconlist.Create(MAKEINTRESOURCE(IDR_IMAGELIST), 16, 1, CLR_DEFAULT);

   // sub-class our m_propertyTree control.
   m_propertyTree.SubclassDlgItem(IDC_OBJECTPROPERTIES, this);
   //m_propertyTree.SetImageList(&iconlist, TVSIL_NORMAL);

	m_propertyTree.SetOwner(this);

	_construct();

	m_propertyTree.Expand(TVI_ROOT, TVE_EXPAND);
	
	m_editor.addMonitor(*this);
	
	CRect savedRect;
	if (theApp.getDialogPosition(savedRect, "ObjectBrowserDialog"))
	{
		MoveWindow(savedRect.left, savedRect.top, savedRect.Width(), savedRect.Height());
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL ObjectPropertiesTreeDialog::PreTranslateMessage(MSG* pMsg) 
{
	CWnd *pMainWnd = GetOwner();

	if	( !(  pMainWnd
			&& accelerators
			&& ::TranslateAccelerator(pMainWnd->m_hWnd, accelerators, pMsg)
			)
		)
	{
		return CDialog::PreTranslateMessage(pMsg);
	}
	else
	{
		return TRUE;
	}

}

LRESULT ObjectPropertiesTreeDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
		case WM_NCACTIVATE:
			wParam=showActive;
			break;
	}
	return CDialog::WindowProc(message, wParam, lParam);
}

void ObjectPropertiesTreeDialog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
   if (m_propertyTree.m_hWnd)
	{
      m_propertyTree.SetWindowPos(0, 0, 0, cx, cy, SWP_NOZORDER);
	}
}
