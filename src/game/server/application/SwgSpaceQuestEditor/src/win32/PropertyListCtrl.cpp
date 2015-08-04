// ======================================================================
//
// PropertyListCtrl.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// Adapted from codeguru.com
//
// ======================================================================

#include "FirstSwgSpaceQuestEditor.h"
#include "PropertyListCtrl.h"

#include "ChildFrame.h"
#include "Configuration.h"
#include "DialogCargo.h"
#include "DialogNavPoint.h"
#include "DialogNavPointList.h"
#include "DialogSpaceMobile.h"
#include "DialogSpaceMobileList.h"
#include "DialogSpawner.h"
#include "DialogSpawnerList.h"
#include "DialogStringEdit.h"
#include "NumberEdit.h"
#include "sharedFoundation/PointerDeleter.h"

// ======================================================================
// PropertyListCtrl::CComboButton
// ======================================================================

class PropertyListCtrl::CComboButton : public CButton
{
public:

	CComboButton();
	virtual ~CComboButton();

	BOOL Create(CRect rect, CWnd * pParent, UINT uID);

public:

	CPen * m_pBkPen;
	CPen * m_pGrayPen;
	CBrush * m_pBkBrush;  
	CBrush * m_pBlackBrush;

public:

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

	//{{AFX_VIRTUAL(CComboButton)
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(CComboButton)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:

	void DrawTriangle(CDC* pDC, CRect Rect);
};

// ----------------------------------------------------------------------

PropertyListCtrl::CComboButton::CComboButton()
{
}

// ----------------------------------------------------------------------

PropertyListCtrl::CComboButton::~CComboButton()
{
	// Delete the objects created
	delete m_pBkBrush;
	delete m_pBlackBrush;
	delete m_pGrayPen;
	delete m_pBkPen;
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(PropertyListCtrl::CComboButton, CButton)
	//{{AFX_MSG_MAP(PropertyListCtrl::CComboButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL PropertyListCtrl::CComboButton::Create(CRect Rect, CWnd * pParent, UINT uID)
{
	// Create the Brushes and Pens
	m_pBkBrush = new CBrush(GetSysColor(COLOR_BTNFACE));
	m_pBkPen = new CPen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
	m_pGrayPen = new CPen(PS_SOLID, 1, RGB(128, 128, 128));
	m_pBlackBrush = new CBrush(RGB(0, 0, 0));

	// Create the CButton
	if (!CButton::Create("", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW, Rect, pParent, uID))
		return FALSE;

	return 0;
}

// ----------------------------------------------------------------------

void PropertyListCtrl::CComboButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC * pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect ButtonRect = lpDrawItemStruct->rcItem;

	// Fill the Background
	CBrush * pOldBrush = (CBrush *) pDC->SelectObject(m_pBkBrush);
	CPen * pOldPen = (CPen *) pDC->SelectObject(m_pBkPen);
	pDC->Rectangle(ButtonRect);

	// Draw the Correct Border
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		pDC->DrawEdge(ButtonRect, EDGE_SUNKEN, BF_RECT);
		ButtonRect.left++;
		ButtonRect.right++;
		ButtonRect.bottom++;
		ButtonRect.top++;
	}

	else
		pDC->DrawEdge(ButtonRect, EDGE_RAISED, BF_RECT);

	// Draw the Triangle
	ButtonRect.left += 3;
	ButtonRect.right -= 4;
	ButtonRect.top += 5;
	ButtonRect.bottom -= 5;
	DrawTriangle(pDC, ButtonRect);

	// Return what was used
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
}

// ----------------------------------------------------------------------

void PropertyListCtrl::CComboButton::DrawTriangle(CDC * pDC, CRect Rect)
{
	POINT ptArray[3];

	// Figure out the Top left
	ptArray[0].x = Rect.left;
	ptArray[0].y = Rect.top;
	ptArray[1].x = Rect.right;
	ptArray[1].y = Rect.top;
	ptArray[2].x = Rect.right - (Rect.Width() / 2);
	ptArray[2].y = Rect.bottom;

	// Select the Brush and Draw the triangle
	IGNORE_RETURN(pDC->SelectObject(m_pBlackBrush));
	pDC->Polygon(ptArray, 3);
}

// ----------------------------------------------------------------------

void PropertyListCtrl::CComboButton::MeasureItem(LPMEASUREITEMSTRUCT /*lpMeasureItemStruct*/)
{
}

// ======================================================================
// PropertyListCtrl::PropertyItem
// ======================================================================

class PropertyListCtrl::PropertyItem
{
public:

	PropertyItem(Configuration::MissionTemplate::PropertyTemplate const * const propertyTemplate, int const type, int const alignment, bool const editCombo, bool const sortData, CString const & toolTip);
	void sortData();

public:

	Configuration::MissionTemplate::PropertyTemplate const * const m_propertyTemplate;
	int	const m_type;
	int const m_alignment;
	bool const m_editCombo;
	bool const m_sortData;
	CString const m_toolTip;
	int	m_width;
	size_t m_propertySelected;
	StringList m_data;
};

// ----------------------------------------------------------------------

PropertyListCtrl::PropertyItem::PropertyItem(Configuration::MissionTemplate::PropertyTemplate const * const propertyTemplate, int const type, int const alignment, bool const editCombo, bool const sortData, CString const & toolTip) :
	m_propertyTemplate(propertyTemplate),
	m_type(type),
	m_alignment(alignment),
	m_editCombo(editCombo),
	m_sortData(sortData),
	m_toolTip(toolTip)
{
}

// ----------------------------------------------------------------------

void PropertyListCtrl::PropertyItem::sortData()
{
	std::sort(m_data.begin(), m_data.end());
}

// ======================================================================
// PropertyListCtrl::PropertyItem
// ======================================================================

PropertyListCtrl::PropertyListCtrl(int const controlId, bool const boldSelection) :
	m_controlId(controlId),
	m_boldSelection(boldSelection),
	m_font(new CFont),
	m_selectedFont(new CFont),
	m_borderColor(RGB(192, 192, 192)),
	m_backgroundColor(GetSysColor(COLOR_WINDOW)),
	m_textColor(GetSysColor(COLOR_WINDOWTEXT)),
	m_backgroundHighlightColor(GetSysColor(COLOR_HIGHLIGHT)),
	m_textHighlightColor(GetSysColor(COLOR_HIGHLIGHTTEXT)),
	m_comboButton(0),
	m_listBox(0),
	m_editCtrl(0),
	m_numberEditCtrl(0),
	m_text(),
	m_propertyItemList(),
	m_currentItem(0),
	m_currentDrawItem(0)
{
	m_widestItem = 0;
	m_backgroundBrush = NULL;
	m_editCtrl = NULL;
	m_numberEditCtrl = NULL;
	m_questButton = NULL;
	m_objectTemplateButton = NULL;
	m_comboButton = NULL;
	m_spaceMobileButton = 0;
	m_spaceMobileListButton = 0;
	m_navPointButton = 0;
	m_navPointListButton = 0;
	m_spawnerButton = 0;
	m_spawnerListButton = 0;
	m_cargoButton = 0;
	m_listBox = NULL;
	m_backgroundHighlightBrush = NULL;
	m_borderPen = NULL;
	m_currentItem = NULL;
	m_currentDrawItem = NULL;

}

// ----------------------------------------------------------------------

PropertyListCtrl::~PropertyListCtrl()
{
	delete m_font;
	delete m_selectedFont;

	if (m_editCtrl)
		delete m_editCtrl;

	if (m_numberEditCtrl)
		delete m_numberEditCtrl;

	if (m_questButton)
		delete m_questButton;

	if (m_objectTemplateButton)
		delete m_objectTemplateButton;

	if (m_spaceMobileButton)
		delete m_spaceMobileButton;

	if (m_spaceMobileListButton)
		delete m_spaceMobileListButton;

	if (m_navPointButton)
		delete m_navPointButton;

	if (m_navPointListButton)
		delete m_navPointListButton;

	if (m_spawnerButton)
		delete m_spawnerButton;

	if (m_spawnerListButton)
		delete m_spawnerListButton;

	if (m_cargoButton)
		delete m_cargoButton;

	if (m_listBox)
		delete m_listBox;

	if (m_comboButton)
		delete m_comboButton;

	if (m_backgroundBrush)
		delete m_backgroundBrush;

	if (m_backgroundHighlightBrush)
		delete m_backgroundHighlightBrush;

	if (m_borderPen)
		delete m_borderPen;

	std::for_each(m_propertyItemList.begin(), m_propertyItemList.end(), PointerDeleter());
	m_propertyItemList.clear();

	delete m_parentInterface;
}

// ----------------------------------------------------------------------

void PropertyListCtrl::setParentInterface(ParentInterface * const parentInterface)
{
	m_parentInterface = parentInterface;
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(PropertyListCtrl, CListBox)
	//{{AFX_MSG_MAP(PropertyListCtrl)
	ON_WM_CREATE()
	ON_CONTROL_REFLECT(LBN_SELCHANGE, OnSelchange)
	ON_CONTROL_REFLECT(LBN_DBLCLK, OnDblclk)
	ON_EN_KILLFOCUS(ID_PROPERTY_TEXT, OnEditLostFocus)
	ON_EN_CHANGE(ID_PROPERTY_TEXT, OnEditChange)
	ON_EN_KILLFOCUS(ID_PROPERTY_INTEGER, OnNumberEditLostFocus)
	ON_EN_CHANGE(ID_PROPERTY_INTEGER, OnNumberEditChange)
	ON_BN_CLICKED(ID_PROPERTY_TEXT, OnTextPropertyClick)
	ON_BN_CLICKED(ID_PROPERTY_TEXT_QUEST, OnQuestPropertyClick)
	ON_BN_CLICKED(ID_PROPERTY_TEXT_OBJECTTEMPLATE, OnObjectTemplatePropertyClick)
	ON_BN_CLICKED(ID_PROPERTY_TEXT_SPACEMOBILE, OnSpaceMobilePropertyClick)
	ON_BN_CLICKED(ID_PROPERTY_TEXT_SPACEMOBILELIST, OnSpaceMobileListPropertyClick)
	ON_BN_CLICKED(ID_PROPERTY_TEXT_NAVPOINT, OnNavPointPropertyClick)
	ON_BN_CLICKED(ID_PROPERTY_TEXT_NAVPOINTLIST, OnNavPointListPropertyClick)
	ON_BN_CLICKED(ID_PROPERTY_TEXT_SPAWNER, OnSpawnerPropertyClick)
	ON_BN_CLICKED(ID_PROPERTY_TEXT_SPAWNERLIST, OnSpawnerListPropertyClick)
	ON_BN_CLICKED(ID_PROPERTY_TEXT_CARGO, OnCargoPropertyClick)
	ON_BN_CLICKED(ID_PROPERTY_COMBO_BTN, OnComboBoxClick)
	ON_LBN_SELCHANGE(ID_PROPERTY_COMBO_LIST, OnSelChange)
	ON_LBN_KILLFOCUS(ID_PROPERTY_COMBO_LIST, OnListboxLostFocus)
	ON_WM_LBUTTONDOWN()
	ON_WM_VSCROLL()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

int PropertyListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create the default font and set it
	m_font->CreateFont(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS, "Verdana");

	// Create the Heading font and set it
	m_selectedFont->CreateFont(14, 0, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS, "Verdana");

	// Create the Border Pen
	m_borderPen = new CPen(PS_SOLID, 1, m_borderColor);

	// Create the Selected Background brush
	m_backgroundHighlightBrush = new CBrush(m_backgroundHighlightColor);
	m_backgroundBrush = new CBrush(m_backgroundColor);

	// Set the row height
	SetItemHeight(-1, 16);
	return 0;
}

// ----------------------------------------------------------------------

void PropertyListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// Make sure its a valid item
	if (lpDrawItemStruct->itemID == LB_ERR)
		return;

	// Obtain the text for this item
	m_text.Empty();
	GetText(lpDrawItemStruct->itemID, m_text);

	// Get the drawing DC
	CDC * pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	// Set the Current member we are drawing
	m_currentDrawItem = m_propertyItemList[lpDrawItemStruct->itemID];

	// Obtain the Item Rect
	CRect ItemRect(lpDrawItemStruct->rcItem);

	// Draw This item
	drawItem(pDC, ItemRect, lpDrawItemStruct->itemState & ODS_SELECTED);
}

// ----------------------------------------------------------------------

void PropertyListCtrl::MeasureItem(LPMEASUREITEMSTRUCT /*lpMeasureItemStruct*/)
{
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnDblclk()
{
	// Get the Course Position
	POINT point;
	GetCursorPos(&point);

	// Convert this rect to coordinates of the desktop
	CRect rect = m_currentRect;
	MapWindowPoints(GetDesktopWindow(), rect);

	// Display the Correct Control
	switch (m_currentItem->m_type)
	{
	case ID_PROPERTY_BOOL:
		// Is the Mouse in this area
		if (rect.PtInRect(point))
		{
			// Reverse the Selection
			m_currentItem->m_propertySelected = m_currentItem->m_propertySelected == 0 ? 1 : 0;

			// Redraw this item
			RedrawWindow();

			// Send the message that a property has changed
			m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);
		}
		break;

	case ID_PROPERTY_TEXT:
		OnTextPropertyClick();
		OnSelchange();
		break;

	case ID_PROPERTY_TEXT_QUEST:
		m_questButton->SetFocus();
		OnQuestPropertyClick();
		break;

	case ID_PROPERTY_TEXT_OBJECTTEMPLATE:
		m_objectTemplateButton->SetFocus();
		OnObjectTemplatePropertyClick();
		break;

	case ID_PROPERTY_TEXT_SPACEMOBILE:
		m_spaceMobileButton->SetFocus();
		OnSpaceMobilePropertyClick();
		break;

	case ID_PROPERTY_TEXT_SPACEMOBILELIST:
		m_spaceMobileListButton->SetFocus();
		OnSpaceMobileListPropertyClick();
		break;

	case ID_PROPERTY_TEXT_NAVPOINT:
		m_navPointButton->SetFocus();
		OnNavPointPropertyClick();
		break;

	case ID_PROPERTY_TEXT_NAVPOINTLIST:
		m_navPointListButton->SetFocus();
		OnNavPointListPropertyClick();
		break;

	case ID_PROPERTY_TEXT_SPAWNER:
		m_spawnerButton->SetFocus();
		OnSpawnerPropertyClick();
		break;

	case ID_PROPERTY_TEXT_SPAWNERLIST:
		m_spawnerListButton->SetFocus();
		OnSpawnerListPropertyClick();
		break;

	case ID_PROPERTY_TEXT_CARGO:
		m_cargoButton->SetFocus();
		OnCargoPropertyClick();
		break;
	}
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnSelchange()
{
	hideControls();

	// Display the Correct Control
	CRect rect = m_currentRect;
	rect.InflateRect(-1, -1);
	switch (m_currentItem->m_type)
	{
	case ID_PROPERTY_TEXT:
		rect.left += 1;
		m_editCtrl->SetWindowText(m_currentItem->m_data[0]);
		m_editCtrl->MoveWindow(rect);
		m_editCtrl->ShowWindow(SW_SHOWNORMAL);
		break;

	case ID_PROPERTY_INTEGER:
		rect.left += 1;
		m_numberEditCtrl->SetWindowText(m_currentItem->m_data[0]);
		m_numberEditCtrl->MoveWindow(rect);
		m_numberEditCtrl->ShowWindow(SW_SHOWNORMAL);
		break;

	case ID_PROPERTY_TEXT_QUEST:
		rect.left = rect.right - 17;
		m_questButton->MoveWindow(rect);
		m_questButton->ShowWindow(SW_SHOWNORMAL);
		break;

	case ID_PROPERTY_TEXT_OBJECTTEMPLATE:
		rect.left = rect.right - 17;
		m_objectTemplateButton->MoveWindow(rect);
		m_objectTemplateButton->ShowWindow(SW_SHOWNORMAL);
		break;

	case ID_PROPERTY_TEXT_SPACEMOBILE:
		rect.left = rect.right - 17;
		m_spaceMobileButton->MoveWindow(rect);
		m_spaceMobileButton->ShowWindow(SW_SHOWNORMAL);
		break;

	case ID_PROPERTY_TEXT_SPACEMOBILELIST:
		rect.left = rect.right - 17;
		m_spaceMobileListButton->MoveWindow(rect);
		m_spaceMobileListButton->ShowWindow(SW_SHOWNORMAL);
		break;

	case ID_PROPERTY_TEXT_NAVPOINT:
		rect.left = rect.right - 17;
		m_navPointButton->MoveWindow(rect);
		m_navPointButton->ShowWindow(SW_SHOWNORMAL);
		break;

	case ID_PROPERTY_TEXT_NAVPOINTLIST:
		rect.left = rect.right - 17;
		m_navPointListButton->MoveWindow(rect);
		m_navPointListButton->ShowWindow(SW_SHOWNORMAL);
		break;

	case ID_PROPERTY_TEXT_SPAWNER:
		rect.left = rect.right - 17;
		m_spawnerButton->MoveWindow(rect);
		m_spawnerButton->ShowWindow(SW_SHOWNORMAL);
		break;

	case ID_PROPERTY_TEXT_SPAWNERLIST:
		rect.left = rect.right - 17;
		m_spawnerListButton->MoveWindow(rect);
		m_spawnerListButton->ShowWindow(SW_SHOWNORMAL);
		break;

	case ID_PROPERTY_TEXT_CARGO:
		rect.left = rect.right - 17;
		m_cargoButton->MoveWindow(rect);
		m_cargoButton->ShowWindow(SW_SHOWNORMAL);
		break;

	case ID_PROPERTY_COMBO_LIST:
		rect.left = rect.right - 17;
		m_comboButton->MoveWindow(rect);
		m_comboButton->ShowWindow(SW_SHOWNORMAL);

		rect.left = m_currentRect.left + 2;
		rect.right -= 17;
		if (m_currentItem->m_editCombo)
		{
			m_editCtrl->SetWindowText(m_currentItem->m_data[m_currentItem->m_propertySelected]);
			m_editCtrl->MoveWindow(rect);
			m_editCtrl->ShowWindow(SW_SHOWNORMAL);
		}

		// Move the Lsit box
		rect.left--;
		rect.right += 18;
		rect.top = rect.bottom;

		// Set the Bottom Height
		if (m_currentItem->m_data.size() > 5)
			rect.bottom += GetItemHeight(0) * 5;
		else
			rect.bottom += GetItemHeight(0) * m_currentItem->m_data.size();
		m_listBox->MoveWindow(rect);

		// Force the Expansion
		OnComboBoxClick();
		break;
	}

	safe_cast<ChildFrame *>(GetParentFrame())->setToolTip(m_currentItem->m_toolTip);
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnEditLostFocus()
{
	// Get the text
	CString text;
	m_editCtrl->GetWindowText(text);

	// Is the current item a text item
	if (m_currentItem->m_type == ID_PROPERTY_TEXT)
	{
		// Did the text change
		if (!m_changed)
			return;

		m_currentItem->m_data[0] = text;

		// Send the message that a property has changed
		m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);
	}

	else
	{
		// Get the window that has the focus now
		if (GetFocus() == m_comboButton || !m_listBox->GetCount())
			return;

		// Did the text change
		if (!m_changed)
			return;

		// Send the message that a property has changed
		m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);

		// Look for this text
		m_changed = false;
		if (m_listBox->FindStringExact(-1, text) != LB_ERR)
			return;

		// Add it and select it
		m_currentItem->m_propertySelected = m_currentItem->m_data.size();
		m_currentItem->m_data.push_back(text);

		// Resort the strings is necessary
		if (m_currentItem->m_sortData)
		{
			m_currentItem->sortData();

			// Search the the string and set its positon the selected one
			m_currentItem->m_propertySelected = std::find(m_currentItem->m_data.begin(), m_currentItem->m_data.end(), text) - m_currentItem->m_data.begin();
		}
	}
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnEditChange()
{
	m_changed = true;
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnNumberEditLostFocus()
{
	// Get the text
	CString text;
	m_numberEditCtrl->GetWindowText(text);

	// Is the current item a text item
	if (m_currentItem->m_type == ID_PROPERTY_INTEGER)
	{
		// Did the text change
		if (!m_changed)
			return;

		m_currentItem->m_data[0] = text;

		// Send the message that a property has changed
		m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);
	}

	else
	{
		// Get the window that has the focus now
		if (GetFocus() == m_comboButton || !m_listBox->GetCount())
			return;

		// Did the text change
		if (!m_changed)
			return;

		// Send the message that a property has changed
		m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);

		// Look for this text
		m_changed = false;
		if (m_listBox->FindStringExact(-1, text) != LB_ERR)
			return;

		// Add it and select it
		m_currentItem->m_propertySelected = m_currentItem->m_data.size();
		m_currentItem->m_data.push_back(text);

		// Resort the strings is necessary
		if (m_currentItem->m_sortData)
		{
			std::sort(m_currentItem->m_data.begin(), m_currentItem->m_data.end());

			// Search the the string and set its positon the selected one
			m_currentItem->m_propertySelected = std::find(m_currentItem->m_data.begin(), m_currentItem->m_data.end(), text) - m_currentItem->m_data.begin();
		}
	}
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnNumberEditChange()
{
	m_changed = true;
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnTextPropertyClick()
{
	// Show the Dialog
	DialogStringEdit dlg(m_text, m_currentItem->m_data[0]);
	if (dlg.DoModal() != IDOK)
		return;

	// Obtain the Path they selected
	m_currentItem->m_data[0] = dlg.getString();

	// Redraw
	RedrawWindow();

	// Send the message that a property has changed
	m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnQuestPropertyClick()
{
	// Show the Dialog
	CFileDialog dlg(TRUE, "*.tab");
	dlg.m_ofn.lpstrInitialDir = Configuration::getServerMissionDataTablePath();
	if (dlg.DoModal() != IDOK)
		return;

	// Obtain the Path they selected
	m_currentItem->m_data[0] = Configuration::extractMissionTemplateType(dlg.GetPathName()) + ':' + Configuration::extractRootName(dlg.GetPathName());

	// Redraw
	RedrawWindow();

	// Send the message that a property has changed
	m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnObjectTemplatePropertyClick()
{
	// Show the Dialog
	CFileDialog dlg(TRUE, "*.iff");
	CString filePath(Configuration::getServerMissionDataTablePath() + (!m_currentItem->m_propertyTemplate->m_propertyData.IsEmpty() ? ("/../" + m_currentItem->m_propertyTemplate->m_propertyData) : "/../object"));
	filePath.Replace('/', '\\');
	filePath.Replace("dsrc", "data");
	dlg.m_ofn.lpstrInitialDir = filePath;
	if (dlg.DoModal() != IDOK)
		return;

	// Obtain the Path they selected
	CString fileName(dlg.GetPathName());
	int const index = fileName.Find("object");
	if (index == -1)
	{
		MessageBox(fileName + " does not appear to be a valid object template");
		return;
	}

	fileName = fileName.Right(fileName.GetLength() - index);
	fileName.Replace('\\', '/');
	m_currentItem->m_data[0] = fileName;

	// Redraw
	RedrawWindow();

	// Send the message that a property has changed
	m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnSpaceMobilePropertyClick()
{
	DialogSpaceMobile dlg(m_currentItem->m_data[0]);
	if (dlg.DoModal() == IDOK)
	{
		// Obtain the Path they selected
		m_currentItem->m_data[0] = dlg.getSpaceMobile();

		// Redraw
		RedrawWindow();

		// Send the message that a property has changed
		m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);
	}
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnSpaceMobileListPropertyClick()
{
	DialogSpaceMobileList dlg(m_currentItem->m_data[0]);
	if (dlg.DoModal() == IDOK)
	{
		// Obtain the Path they selected
		m_currentItem->m_data[0] = dlg.getSpaceMobileList();

		// Redraw
		RedrawWindow();

		// Send the message that a property has changed
		m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);
	}
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnNavPointPropertyClick()
{
	DialogNavPoint dlg(m_currentItem->m_data[0]);
	if (dlg.DoModal() == IDOK)
	{
		m_currentItem->m_data[0] = dlg.getNavPoint();

		// Redraw
		RedrawWindow();

		// Send the message that a property has changed
		m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);
	}
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnNavPointListPropertyClick()
{
	DialogNavPointList dlg(m_currentItem->m_data[0]);
	if (dlg.DoModal() == IDOK)
	{
		m_currentItem->m_data[0] = dlg.getNavPointList();

		// Redraw
		RedrawWindow();

		// Send the message that a property has changed
		m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);
	}
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnSpawnerPropertyClick()
{
	DialogSpawner dlg(m_currentItem->m_data[0]);
	if (dlg.DoModal() == IDOK)
	{
		m_currentItem->m_data[0] = dlg.getSpawner();

		// Redraw
		RedrawWindow();

		// Send the message that a property has changed
		m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);
	}
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnSpawnerListPropertyClick()
{
	DialogSpawnerList dlg(m_currentItem->m_data[0]);
	if (dlg.DoModal() == IDOK)
	{
		m_currentItem->m_data[0] = dlg.getSpawnerList();

		// Redraw
		RedrawWindow();

		// Send the message that a property has changed
		m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);
	}
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnCargoPropertyClick()
{
	if (!m_currentItem->m_propertyTemplate)
	{
		MessageBox("Cargo property has no missing property template.  See asommers.");
		return;
	}

	//-- Find index of the property with the name m_propertyData
	CString propertyName("<None>");
	getPropertyName(GetCurSel(), propertyName);

	//-- Gather space mobile list
	CString spaceMobileProperty;
	StringList spaceMobileList;
	{
		CString const spaceMobilePropertyName = !m_currentItem->m_propertyTemplate->m_propertyData.IsEmpty() ? m_currentItem->m_propertyTemplate->m_propertyData : "<None>";
		int index = FindStringExact(0, spaceMobilePropertyName);
		if (index == LB_ERR)
		{
			index = FindStringExact(0, "*" + spaceMobilePropertyName);
			if (index == LB_ERR)
			{
				MessageBox("Cargo property " + propertyName + " could not find space mobile property with property name " + spaceMobilePropertyName + ".  Check the SwgSpaceQuestEditor.ini file and make sure the property type PT_cargo specifies the space mobile name property name to search.");
				return;
			}
		}		

		if (!getProperty(index, spaceMobileProperty))
		{
			MessageBox("ERROR(3): see asommers.");
			return;
		}

		//-- Make sure the space mobile is valid
		Configuration::unpackString(spaceMobileProperty, spaceMobileList, '|');
		if (spaceMobileList.empty())
		{
			MessageBox("Cargo property " + propertyName + " uses invalid space mobile <None>");
			return;
		}
	}

	//-- Create set of cargo specified by spaceMobileList
	StringSet cargoSet;
	for (uint i = 0; i < spaceMobileList.size(); ++i)
	{
		CString const spaceMobile = spaceMobileList[i];

		if (!Configuration::isValidSpaceMobile(spaceMobile))
		{
			MessageBox("Cargo property " + propertyName + " uses invalid space mobile " + spaceMobile);
			return;
		}

		//-- Grab space mobile cargo name
		CString const cargoName = Configuration::getSpaceMobileCargoName(spaceMobile);
		if (!cargoName.IsEmpty())
		{
			//-- Make sure the cargo is valid
			StringList const * const stringList = Configuration::getCargo(cargoName);
			if (stringList)
				cargoSet.insert(stringList->begin(), stringList->end());
			else
			{
				MessageBox("Space mobile " + spaceMobile + " refers to invalid cargo name " + cargoName);
				return;
			}
		}
	}

	if (cargoSet.empty())
	{
		MessageBox("Could not extract cargo contents from any of the following space mobiles: " + spaceMobileProperty);
		return;
	}

	//-- Extract the cargo name from the 
	DialogCargo dlg(cargoSet, m_currentItem->m_data[0]);
	if (dlg.DoModal() == IDOK)
	{
		m_currentItem->m_data[0] = dlg.getResult();

		// Redraw
		RedrawWindow();

		// Send the message that a property has changed
		m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);
	}
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnComboBoxClick()
{
	// Add the items
	m_listBox->ResetContent();

	// Loop for all items
	for (size_t i = 0; i < m_currentItem->m_data.size(); ++i)
		m_listBox->AddString(m_currentItem->m_data[i]);

	// Select the correct item
	m_listBox->SetCurSel(m_currentItem->m_propertySelected);
	m_listBox->SetTopIndex(m_currentItem->m_propertySelected);

	// Show the List box
	m_listBox->ShowWindow(SW_NORMAL);
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnSelChange()
{
	// Set the new current item
	m_currentItem->m_propertySelected = m_listBox->GetCurSel();

	// Hide the Windows
	m_listBox->ShowWindow(SW_HIDE);

	if (m_currentItem->m_editCombo)
		m_editCtrl->SetWindowText(m_currentItem->m_data[m_currentItem->m_propertySelected]);
	else
		RedrawWindow();

	// Send the message that a property has changed
	m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);
	m_comboButton->SetFocus();
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnListboxLostFocus()
{
	m_listBox->ShowWindow(SW_HIDE);
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	// is there an item at this point
	BOOL outside;
	IGNORE_RETURN(ItemFromPoint(point, outside));

	// Is this outside the client
	if (outside)
		hideControls();

	CListBox::OnLButtonDown(nFlags, point);
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar)
{
	// Hide the Controls
	hideControls();

	CListBox::OnVScroll(nSBCode, nPos, pScrollBar);
}

// ----------------------------------------------------------------------

bool PropertyListCtrl::addString(Configuration::MissionTemplate::PropertyTemplate const * const propertyTemplate, CString const & text, int type, CString const & csData, CString const & toolTip, int propertySelected, int const alignment, bool const editCombo, bool const sortData)
{
	// Is this a valid Control type
	if (type >= ID_PROPERTY_LAST)
		return false;

	// Create a new Structure to hold it
	PropertyItem * const propertyItem = new PropertyItem(propertyTemplate, type, alignment, editCombo, sortData, toolTip);

	// Calculate the Width of the string based on the font set
	CDC * const pDC = GetDC();
	pDC->SelectObject(m_selectedFont);
	CSize size = pDC->GetTextExtent(text);
	if (size.cx + 10 > m_widestItem)
		m_widestItem = size.cx + 10;

	ReleaseDC(pDC);
	propertyItem->m_width = size.cx;
	propertyItem->m_propertySelected = propertySelected;

	// Set Property
	if (!setProperty(propertyItem, csData))
	{
		DEBUG_FATAL(true, ("SetProperty failed"));
		delete propertyItem;
		return false;
	}

	// Is the item set bigger than the number of properties
	if (propertyItem->m_propertySelected >= propertyItem->m_data.size())
	{
		DEBUG_FATAL(true, ("Property select failed"));
		delete propertyItem;
		return false;
	}

	// Add to the list
	m_propertyItemList.push_back(propertyItem);

	// Add the string to the list box
	IGNORE_RETURN(CListBox::AddString(text));

	// Create the Control if Needed
	createControl(type);

	return true;
}

// ----------------------------------------------------------------------

void PropertyListCtrl::drawItem(CDC * const pDC, CRect itemRect, bool const selected)
{
	/////////////////////////////////////////
	// Paint the Background rectangle (Property Value)
	pDC->SelectObject(m_backgroundBrush);
	pDC->SelectObject(m_borderPen);

	// Draw the Rectangle
	itemRect.left = m_widestItem - 1;
	itemRect.top--;
	itemRect.right++;
	pDC->Rectangle(itemRect);
	CRect const originalRect = itemRect;

	/////////////////////////////////////////
	// Draw the Property Text
	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(m_backgroundBrush);
	pDC->SelectObject(m_font);
	pDC->SetTextColor(m_textColor);
	drawPropertyText(pDC, itemRect);

	/////////////////////////////////////////
	// Paint the Background rectangle (Property Name)
	if (selected)
		pDC->SelectObject(m_backgroundHighlightBrush);

	// Draw the Rectangle
	itemRect.right = m_widestItem;
	itemRect.left = -1;
	pDC->Rectangle(itemRect);

	/////////////////////////////////////////
	// Paint the Property name Text
	// Is this item selected?
	if (selected)
	{
		if (m_boldSelection)
			pDC->SelectObject(m_selectedFont);

		pDC->SetTextColor(m_textHighlightColor);
		m_currentItem = m_currentDrawItem;
		m_currentRect = originalRect;
	}

	// Draw the Text
	itemRect.left += 6;
	itemRect.right -= 5;
	pDC->DrawText(m_text, m_text.GetLength(), itemRect, DT_SINGLELINE | DT_VCENTER | m_currentDrawItem->m_alignment);
}

// ----------------------------------------------------------------------

void PropertyListCtrl::drawPropertyText(CDC * const pDC, CRect itemRect)
{
	itemRect.left += 5;

	switch (m_currentDrawItem->m_type)
	{
	case ID_PROPERTY_BOOL:
	case ID_PROPERTY_COMBO_LIST:
		pDC->DrawText(m_currentDrawItem->m_data[m_currentDrawItem->m_propertySelected], m_currentDrawItem->m_data[m_currentDrawItem->m_propertySelected].GetLength(), itemRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
		break;

	case ID_PROPERTY_STATIC:
	case ID_PROPERTY_TEXT:
	case ID_PROPERTY_INTEGER:
	case ID_PROPERTY_TEXT_QUEST:
	case ID_PROPERTY_TEXT_OBJECTTEMPLATE:
	case ID_PROPERTY_TEXT_SPACEMOBILE:
	case ID_PROPERTY_TEXT_SPACEMOBILELIST:
	case ID_PROPERTY_TEXT_NAVPOINT:
	case ID_PROPERTY_TEXT_NAVPOINTLIST:
	case ID_PROPERTY_TEXT_SPAWNER:
	case ID_PROPERTY_TEXT_SPAWNERLIST:
	case ID_PROPERTY_TEXT_CARGO:
		pDC->DrawText(m_currentDrawItem->m_data[0], m_currentDrawItem->m_data[0].GetLength(), itemRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
		break;
	}
}

// ----------------------------------------------------------------------

void PropertyListCtrl::createControl(int const type)
{
	switch (type)
	{
	case ID_PROPERTY_TEXT:
		{
			if (!m_editCtrl)
			{
				m_editCtrl = new CEdit();
				m_editCtrl->Create(WS_CHILD | ES_AUTOHSCROLL | ES_LEFT, CRect(0, 0, 100, 100), this, ID_PROPERTY_TEXT);
				m_editCtrl->SetFont(m_font);
			}
		}
		break;

	case ID_PROPERTY_INTEGER:
		{
			if (!m_numberEditCtrl)
			{
				m_numberEditCtrl = new NumberEdit(true, true);
				m_numberEditCtrl->Create(WS_CHILD | ES_AUTOHSCROLL | ES_LEFT, CRect(0, 0, 100, 100), this, ID_PROPERTY_INTEGER);
				m_numberEditCtrl->SetFont(m_font);
			}
		}
		break;

	case ID_PROPERTY_TEXT_QUEST:
		{
			if (!m_questButton)
			{
				m_questButton = new CButton();
				m_questButton->Create("...", WS_CHILD | BS_PUSHBUTTON, CRect(0, 0, 100, 100), this, ID_PROPERTY_TEXT_QUEST);
				m_questButton->SetFont(m_font);
			}
		}
		break;

	case ID_PROPERTY_TEXT_OBJECTTEMPLATE:
		{
			if (!m_objectTemplateButton)
			{
				m_objectTemplateButton = new CButton();
				m_objectTemplateButton->Create("...", WS_CHILD | BS_PUSHBUTTON, CRect(0, 0, 100, 100), this, ID_PROPERTY_TEXT_OBJECTTEMPLATE);
				m_objectTemplateButton->SetFont(m_font);
			}
		}
		break;

	case ID_PROPERTY_TEXT_SPACEMOBILE:
		{
			if (!m_spaceMobileButton)
			{
				m_spaceMobileButton = new CButton();
				m_spaceMobileButton->Create("...", WS_CHILD | BS_PUSHBUTTON, CRect(0, 0, 100, 100), this, ID_PROPERTY_TEXT_SPACEMOBILE);
				m_spaceMobileButton->SetFont(m_font);
			}
		}
		break;

	case ID_PROPERTY_TEXT_SPACEMOBILELIST:
		{
			if (!m_spaceMobileListButton)
			{
				m_spaceMobileListButton = new CButton();
				m_spaceMobileListButton->Create("...", WS_CHILD | BS_PUSHBUTTON, CRect(0, 0, 100, 100), this, ID_PROPERTY_TEXT_SPACEMOBILELIST);
				m_spaceMobileListButton->SetFont(m_font);
			}
		}
		break;

	case ID_PROPERTY_TEXT_NAVPOINT:
		{
			if (!m_navPointButton)
			{
				m_navPointButton = new CButton();
				m_navPointButton->Create("...", WS_CHILD | BS_PUSHBUTTON, CRect(0, 0, 100, 100), this, ID_PROPERTY_TEXT_NAVPOINT);
				m_navPointButton->SetFont(m_font);
			}
		}
		break;

	case ID_PROPERTY_TEXT_NAVPOINTLIST:
		{
			if (!m_navPointListButton)
			{
				m_navPointListButton = new CButton();
				m_navPointListButton->Create("...", WS_CHILD | BS_PUSHBUTTON, CRect(0, 0, 100, 100), this, ID_PROPERTY_TEXT_NAVPOINTLIST);
				m_navPointListButton->SetFont(m_font);
			}
		}
		break;

	case ID_PROPERTY_TEXT_SPAWNER:
		{
			if (!m_spawnerButton)
			{
				m_spawnerButton = new CButton();
				m_spawnerButton->Create("...", WS_CHILD | BS_PUSHBUTTON, CRect(0, 0, 100, 100), this, ID_PROPERTY_TEXT_SPAWNER);
				m_spawnerButton->SetFont(m_font);
			}
		}
		break;

	case ID_PROPERTY_TEXT_SPAWNERLIST:
		{
			if (!m_spawnerListButton)
			{
				m_spawnerListButton = new CButton();
				m_spawnerListButton->Create("...", WS_CHILD | BS_PUSHBUTTON, CRect(0, 0, 100, 100), this, ID_PROPERTY_TEXT_SPAWNERLIST);
				m_spawnerListButton->SetFont(m_font);
			}
		}
		break;

	case ID_PROPERTY_TEXT_CARGO:
		{
			if (!m_cargoButton)
			{
				m_cargoButton = new CButton();
				m_cargoButton->Create("...", WS_CHILD | BS_PUSHBUTTON, CRect(0, 0, 100, 100), this, ID_PROPERTY_TEXT_CARGO);
				m_cargoButton->SetFont(m_font);
			}
		}
		break;

	case ID_PROPERTY_COMBO_LIST:
		{
			if (!m_editCtrl)
			{
				m_editCtrl = new CEdit();
				m_editCtrl->Create(WS_CHILD | ES_AUTOHSCROLL | ES_LEFT, CRect(0, 0, 100, 100), this, ID_PROPERTY_TEXT);
				m_editCtrl->SetFont(m_font);
			}

			if (!m_listBox)
			{
				m_listBox = new CListBox();
				m_listBox->Create(WS_CHILD | WS_BORDER | LBS_NOTIFY | WS_VSCROLL | LBS_HASSTRINGS, CRect(0, 0, 100, 100), this, ID_PROPERTY_COMBO_LIST);
				m_listBox->SetFont(m_font);

				m_comboButton = new CComboButton();
				m_comboButton->Create(CRect(0, 0, 0, 0), this, ID_PROPERTY_COMBO_BTN);
				m_comboButton->SetFont(m_font);
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------

bool PropertyListCtrl::setProperty(PropertyItem * const propertyItem, CString const & data)
{
	switch (propertyItem->m_type)
	{
	case ID_PROPERTY_BOOL:
		{
			parseProperties(propertyItem, data);

			// Is the item selected more than items in the array?
			if (propertyItem->m_data.size() != 2)
				return FALSE;
		}
		break;

	case ID_PROPERTY_TEXT_QUEST:
	case ID_PROPERTY_TEXT_OBJECTTEMPLATE:
	case ID_PROPERTY_TEXT:
	case ID_PROPERTY_TEXT_SPACEMOBILE:
	case ID_PROPERTY_TEXT_SPACEMOBILELIST:
	case ID_PROPERTY_TEXT_NAVPOINT:
	case ID_PROPERTY_TEXT_NAVPOINTLIST:
	case ID_PROPERTY_TEXT_SPAWNER:
	case ID_PROPERTY_TEXT_SPAWNERLIST:
	case ID_PROPERTY_TEXT_CARGO:
	case ID_PROPERTY_STATIC:
	case ID_PROPERTY_INTEGER:
		propertyItem->m_data.push_back(data);
		break;

	case ID_PROPERTY_COMBO_LIST:
		{
			parseProperties(propertyItem, data);

			if (propertyItem->m_sortData)
				propertyItem->sortData();
		}
		break;
	}

	return TRUE;
}

// ----------------------------------------------------------------------

void PropertyListCtrl::parseProperties(PropertyItem * const propertyItem, CString const & data)
{
	Configuration::unpackString(data, propertyItem->m_data, '!');
}

// ----------------------------------------------------------------------

void PropertyListCtrl::hideControls()
{
	// Hide the controls
	if (m_editCtrl)
		m_editCtrl->ShowWindow(SW_HIDE);
	
	if (m_numberEditCtrl)
		m_numberEditCtrl->ShowWindow(SW_HIDE);
	
	if (m_questButton)
		m_questButton->ShowWindow(SW_HIDE);
	
	if (m_objectTemplateButton)
		m_objectTemplateButton->ShowWindow(SW_HIDE);
	
	if (m_spaceMobileButton)
		m_spaceMobileButton->ShowWindow(SW_HIDE);
	
	if (m_spaceMobileListButton)
		m_spaceMobileListButton->ShowWindow(SW_HIDE);
	
	if (m_navPointButton)
		m_navPointButton->ShowWindow(SW_HIDE);
	
	if (m_navPointListButton)
		m_navPointListButton->ShowWindow(SW_HIDE);
	
	if (m_spawnerButton)
		m_spawnerButton->ShowWindow(SW_HIDE);
	
	if (m_spawnerListButton)
		m_spawnerListButton->ShowWindow(SW_HIDE);
	
	if (m_cargoButton)
		m_cargoButton->ShowWindow(SW_HIDE);
	
	if (m_listBox)
		m_listBox->ShowWindow(SW_HIDE);

	if (m_comboButton)
		m_comboButton->ShowWindow(SW_HIDE);
}

// ----------------------------------------------------------------------

bool PropertyListCtrl::getPropertyName(int const index, CString & propertyName) const
{
	if (index < 0 || index >= GetCount())
		return false;

	GetText(index, propertyName);

	return true;
}

// ----------------------------------------------------------------------

bool PropertyListCtrl::getProperty(int const index, CString & value) const
{
	// is the item to high
	if (index < 0 || index >= GetCount())
		return false;

	// Make sure this item is the correct type
	PropertyItem * const propertyItem = m_propertyItemList[index];
	switch (propertyItem->m_type)
	{
	default:
	case ID_PROPERTY_COMBO_BTN:
		return false;

	case ID_PROPERTY_COMBO_LIST:
		value = propertyItem->m_data[propertyItem->m_propertySelected];
		break;

	case ID_PROPERTY_BOOL:
		value = propertyItem->m_propertySelected != 0 ? "1" : "0";
		break;

	case ID_PROPERTY_TEXT:
	case ID_PROPERTY_INTEGER:
	case ID_PROPERTY_TEXT_SPACEMOBILE:
	case ID_PROPERTY_TEXT_SPACEMOBILELIST:
	case ID_PROPERTY_TEXT_SPAWNER:
	case ID_PROPERTY_TEXT_SPAWNERLIST:
	case ID_PROPERTY_TEXT_CARGO:
	case ID_PROPERTY_TEXT_NAVPOINT:
	case ID_PROPERTY_TEXT_NAVPOINTLIST:
	case ID_PROPERTY_TEXT_QUEST:
	case ID_PROPERTY_TEXT_OBJECTTEMPLATE:
	case ID_PROPERTY_STATIC:
		value = propertyItem->m_data[0];
		break;
	}

	return true;
}

// ----------------------------------------------------------------------

void PropertyListCtrl::removeString(CString const & string)
{
	int const selection = FindStringExact(0, string);
	if (selection != LB_ERR)
	{
		delete m_propertyItemList[selection];
		m_propertyItemList.erase(m_propertyItemList.begin() + selection);
		DeleteString(selection);
	}
}

// ----------------------------------------------------------------------

void PropertyListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if (nChar == 46)
	{
		if (m_currentItem)
		{
			if (m_currentItem->m_type == ID_PROPERTY_TEXT ||
				m_currentItem->m_type == ID_PROPERTY_TEXT_SPACEMOBILE ||
				m_currentItem->m_type == ID_PROPERTY_TEXT_SPACEMOBILELIST ||
				m_currentItem->m_type == ID_PROPERTY_TEXT_NAVPOINT ||
				m_currentItem->m_type == ID_PROPERTY_TEXT_NAVPOINTLIST ||
				m_currentItem->m_type == ID_PROPERTY_TEXT_SPAWNER ||
				m_currentItem->m_type == ID_PROPERTY_TEXT_SPAWNERLIST ||
				m_currentItem->m_type == ID_PROPERTY_TEXT_CARGO ||
				m_currentItem->m_type == ID_PROPERTY_TEXT_OBJECTTEMPLATE ||
				m_currentItem->m_type == ID_PROPERTY_TEXT_QUEST)
			{
				CString propertyName;
				getPropertyName(GetCurSel(), propertyName);

				if (MessageBox("Delete data for property '" + propertyName + "'?", 0, MB_YESNO) == IDYES)
				{
					m_currentItem->m_data[0] = "";

					if (m_currentItem->m_type == ID_PROPERTY_TEXT)
						m_editCtrl->SetWindowText(m_currentItem->m_data[0]);

					// Redraw
					RedrawWindow();

					// Send the message that a property has changed
					m_parentInterface->onPropertyChanged(m_controlId << 16 | GetCurSel(), m_currentItem->m_type);
				}
			}
		}
	}

	CListBox::OnKeyDown(nChar, nRepCnt, nFlags);
}

// ======================================================================

