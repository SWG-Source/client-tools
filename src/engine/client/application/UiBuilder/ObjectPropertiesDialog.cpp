// ObjectPropertiesDialog.cpp : implementation file
//

#include "FirstUiBuilder.h"
#include "ObjectEditor.h"
#include "ObjectPropertiesDialog.h"
#include "PropertiesPage.h"
#include "UserWindowsMessages.h"

#include "UIBuilder.h"

extern CUiBuilderApp theApp;

#define ENABLE_STACKED_TABS TRUE


static inline 
PropertiesPage *downcast(ObjectPropertiesEditor::PropertyCategory *p) { return static_cast<PropertiesPage *>(p); }

/////////////////////////////////////////////////////////////////////////////
// ObjectPropertiesDialog

IMPLEMENT_DYNAMIC(ObjectPropertiesDialog, CPropertySheet)

ObjectPropertiesDialog::ObjectPropertiesDialog(ObjectEditor &i_editor, UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage),
	ObjectPropertiesEditor(i_editor),
	accelerators(0),
	tabControlID(0),
	showActive(false)
{
	_construct();
}

// =====================================================================

ObjectPropertiesDialog::ObjectPropertiesDialog(ObjectEditor &i_editor, LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
:	CPropertySheet(pszCaption, pParentWnd, iSelectPage),
	ObjectPropertiesEditor(i_editor),
	accelerators(0),
	tabControlID(0),
	showActive(false)
{
	_construct();
}

// =====================================================================

ObjectPropertiesDialog::~ObjectPropertiesDialog()
{
}

// =====================================================================

void ObjectPropertiesDialog::destroy()
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

// =====================================================================

ObjectPropertiesDialog::PropertyCategory *ObjectPropertiesDialog::_createCategory(int i)
{
	PropertiesPage *returnValue = new PropertiesPage(m_editor, UIPropertyCategories::Category(i));
	AddPage(returnValue);
	return returnValue;
}

// ==========================================================

void ObjectPropertiesDialog::saveUserPreferences()
{
	if (IsWindowVisible())
	{
		CRect windowRect;
		GetWindowRect(windowRect);
		theApp.saveDialogPosition("ObjectPropertiesDialog", windowRect);
	}
}

// ==========================================================

void ObjectPropertiesDialog::setActiveAppearance(bool i_showActive)
{
	if (i_showActive!=showActive)
	{
		showActive=i_showActive;
		PostMessage(WM_NCACTIVATE, showActive);
	}
}

// =====================================================================

// lock the dialog to temporarily prevent redraws during periods where numerous events will be generated.
void ObjectPropertiesDialog::_lock()
{
	CTabCtrl *tc = GetTabControl();
	tc->SetRedraw(false);

	for (int i=0;i<UIPropertyCategories::C_NUM_CATEGORIES;i++)
	{
		if (downcast(m_propertyCategories[i])->m_hWnd)
		{
			downcast(m_propertyCategories[i])->SetRedraw(false);
		}
	}
}

// =====================================================================

// unlock the dialog to allow redraws.
void ObjectPropertiesDialog::_unlock()
{
	CTabCtrl *tc = GetTabControl();
	tc->SetRedraw(true);

	_retextDialog();

	int currentSelection = GetActiveIndex();
	PropertiesPage *pp = downcast(m_propertyCategories[currentSelection]);
	if (pp->m_hWnd)
	{
		pp->SetRedraw(true);
		pp->Invalidate();
	}
}

// =====================================================================

void ObjectPropertiesDialog::onEditReset()
{
	ObjectPropertiesEditor::onEditReset();
	_retextAllTabs();
	_retextDialog();
}

// =====================================================================

void ObjectPropertiesDialog::onEditInsertSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling)
{
	ObjectPropertiesEditor::onEditInsertSubtree(subTree, previousSibling);
	_retextAllTabs();
	_retextDialog();
}

// =====================================================================

void ObjectPropertiesDialog::onEditRemoveSubtree(UIBaseObject &subTree)
{
	ObjectPropertiesEditor::onEditRemoveSubtree(subTree);
	_retextAllTabs();
	_retextDialog();
}

// =====================================================================

void ObjectPropertiesDialog::onEditMoveSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling, UIBaseObject *oldParent)
{
	ObjectPropertiesEditor::onEditMoveSubtree(subTree, previousSibling, oldParent);
	_retextAllTabs();
	_retextDialog();
}

// =====================================================================

void ObjectPropertiesDialog::onEditSetObjectProperty(UIBaseObject &object, const char *i_propertyName)
{
	ObjectPropertiesEditor::onEditSetObjectProperty(object, i_propertyName);
}

// =====================================================================

void ObjectPropertiesDialog::onSelect(UIBaseObject &object, bool isSelected)
{
	ObjectPropertiesEditor::onSelect(object, isSelected);
	_retextAllTabs();
	_retextDialog();
}

// =====================================================================

void ObjectPropertiesDialog::_retextTab(CTabCtrl *tabControl, int tabSelection)
{
	TCITEM tcItem;
	memset(&tcItem, 0, sizeof(tcItem));
	tcItem.mask = TCIF_TEXT;
	tcItem.lParam = 0;
	char tabText[256];
	tabText[0]=0;
	tcItem.pszText = tabText;
	tcItem.cchTextMax = sizeof(tabText);

	const char *categoryName = UIPropertyCategories::s_propertyCategoryNames[tabSelection].c_str();
	PropertiesPage *pp = downcast(m_propertyCategories[tabSelection]);
	if (pp)
	{
		pp->getLabelText(tabText, sizeof(tabText));
		sprintf(tabText, "%s (%i)", categoryName, pp->getSelectionCount());
	}
	else
	{
		strcpy(tabText, categoryName);
	}

	tabControl->SetItem(tabSelection, &tcItem);
}

// =====================================================================

void ObjectPropertiesDialog::_retextAllTabs()
{
	if (!m_hWnd)
	{
		return;
	}
	CTabCtrl *tc = GetTabControl();
	for (int i=0;i<UIPropertyCategories::C_NUM_CATEGORIES;i++)
	{
		_retextTab(tc, i);
	}
}

// =====================================================================

void ObjectPropertiesDialog::_retextSelectedTab()
{
	CTabCtrl *tc = GetTabControl();
	_retextTab(tc, tc->GetCurSel());
}

// =====================================================================

void ObjectPropertiesDialog::_retextDialog()
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

// =====================================================================

BEGIN_MESSAGE_MAP(ObjectPropertiesDialog, CPropertySheet)
	//{{AFX_MSG_MAP(ObjectPropertiesDialog)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ObjectPropertiesDialog message handlers

BOOL ObjectPropertiesDialog::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	
	m_editor.addMonitor(*this);

	CTabCtrl *tabControl = GetTabControl();
	if (tabControl)
	{
		// I think this is always 12320
		tabControlID = tabControl->GetDlgCtrlID();
	}

	_retextAllTabs();

	CRect savedRect;
	if (theApp.getDialogPosition(savedRect, "ObjectPropertiesDialog"))
	{
		SetWindowPos(0, savedRect.left, savedRect.top, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
	}

	return bResult;
}

void ObjectPropertiesDialog::PostNcDestroy() 
{
	m_editor.removeMonitor(*this);
	delete this;
}

BOOL ObjectPropertiesDialog::OnNotify(WPARAM controlID, LPARAM lParam, LRESULT* pResult) 
{
	if (int(controlID)==tabControlID)
	{
		NMHDR *hdr = (NMHDR *)lParam;
		if (hdr->code==TCN_SELCHANGING)
		{
		}
		else if (hdr->code==TCN_SELCHANGE)
		{
			_retextSelectedTab();
		}
	}
	
	return CPropertySheet::OnNotify(controlID, lParam, pResult);
}

int ObjectPropertiesDialog::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
		return -1;

	EnableStackedTabs(ENABLE_STACKED_TABS);
	
	return 0;
}

BOOL ObjectPropertiesDialog::PreTranslateMessage(MSG* pMsg) 
{
	CWnd *pMainWnd = GetOwner();

	if	( !(  pMainWnd
			&& accelerators
			&& ::TranslateAccelerator(pMainWnd->m_hWnd, accelerators, pMsg)
			)
		)
	{
		return CPropertySheet::PreTranslateMessage(pMsg);
	}
	else
	{
		return TRUE;
	}
}

LRESULT ObjectPropertiesDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
		case WM_NCACTIVATE:
			wParam=showActive;
			break;
	}
	return CPropertySheet::WindowProc(message, wParam, lParam);
}

void ObjectPropertiesDialog::OnClose() 
{
	CWnd *owner = GetOwner();
	if (owner)
	{
		owner->SendMessage(WM_closePropertiesDialog, 0, 0);
	}
}
