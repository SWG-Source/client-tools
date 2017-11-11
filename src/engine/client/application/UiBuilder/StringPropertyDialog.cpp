// StringPropertyDialog.cpp : implementation file
//

#include "FirstUiBuilder.h"
#include "StringPropertyDialog.h"

/////////////////////////////////////////////////////////////////////////////
// StringPropertyDialog dialog


StringPropertyDialog::StringPropertyDialog(CWnd* pParent, CPoint anchorPosition, const char *i_initialValue, FormatRestriction i_textFormat)
:	PropertyDialog(StringPropertyDialog::IDD, anchorPosition, pParent),
	m_textFormat(i_textFormat)
{
	//{{AFX_DATA_INIT(StringPropertyDialog)
	m_stringProperty = i_initialValue ? i_initialValue : _T("");
	//}}AFX_DATA_INIT

	_initStringProperty(i_initialValue);
}

void StringPropertyDialog::_initStringProperty(const char *i_value)
{
	if (!i_value)
	{
		m_stringProperty=_T("");
		return;
	}

	char temp[4096];
	char *diter=temp;
	const char *iter;
	for (iter=i_value;*iter;)
	{
		if (*iter=='\r')
		{
			iter++;
		}
		else if (*iter=='\n')
		{
			*diter++='\r';
			*diter++='\n';
			iter++;
		}
		else
		{
			*diter++=*iter++;
		}
	}
	*diter++=0;

	m_stringProperty = temp;
}

void StringPropertyDialog::getStringProperty(CString &o_value)
{
	const char *const value = m_stringProperty;
	if (!value)
	{
		o_value.Empty();
		return;
	}

	char temp[4096];
	char *diter=temp;
	for (const char *iter=value;*iter;)
	{
		if (*iter=='\r')
		{
			iter++;
		}
		else
		{
			*diter++=*iter++;
		}
	}
	*diter++=0;

	o_value = temp;
}

void StringPropertyDialog::_positionControls()
{
	if (m_editControl.m_hWnd)
	{
		CRect clientRect;
		GetClientRect(&clientRect);

		CRect editRect;
		editRect.top    = clientRect.top;
		editRect.left   = clientRect.left;
		editRect.bottom = clientRect.bottom - 18;
		editRect.right  = clientRect.right-1;
		m_editControl.MoveWindow(&editRect, TRUE);

		CRect okRect;
		okRect.top    = editRect.bottom;
		okRect.bottom = clientRect.bottom;
		okRect.left   = clientRect.left;
		okRect.right  = clientRect.Width()/2;
		m_okButton.MoveWindow(&okRect, TRUE);

		CRect cancelRect;
		cancelRect.top    = okRect.top;
		cancelRect.bottom = okRect.bottom;
		cancelRect.left   = clientRect.left + clientRect.Width()/2;
		cancelRect.right  = clientRect.right-1;
		m_cancelButton.MoveWindow(&cancelRect, TRUE);
	}
}

void StringPropertyDialog::_getStartupRect(CRect &o_rect)
{
	const int height = 110;
	const int width = 250;

	int captionHeight = GetSystemMetrics(SM_CYSMCAPTION);
	o_rect.top = m_anchorPosition.y - captionHeight - 8;
	o_rect.left = m_anchorPosition.x - 8;
	o_rect.bottom = height;
	o_rect.right = width; 
}

/////////////////////////////////////////////////////////////////////////////

void StringPropertyDialog::DoDataExchange(CDataExchange* pDX)
{
	PropertyDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(StringPropertyDialog)
	DDX_Control(pDX, IDOK, m_okButton);
	DDX_Control(pDX, IDCANCEL, m_cancelButton);
	DDX_Control(pDX, IDC_STRING_PROPERTY, m_editControl);
	DDX_Text(pDX, IDC_STRING_PROPERTY, m_stringProperty);
	DDV_MaxChars(pDX, m_stringProperty, 1024);
	//}}AFX_DATA_MAP
/*
	switch (m_textFormat)
	{
	case FR_AnyText:
	{
		DDX_Text(pDX, IDC_STRING_PROPERTY, m_stringProperty);
		DDV_MaxChars(pDX, m_stringProperty, 64);
	} break;
	case FR_Integer:
	{
		DDX_Text(pDX, IDC_STRING_PROPERTY, m_stringProperty);
		if (pDX->m_bSaveAndValidate)
		{
			if (m_stringProperty.IsEmpty())
			{
				m_stringProperty="0";
			}
			else
			{
				// copy integer from dialog
				char temp[16];
				int integerProperty = atoi(m_stringProperty);
				m_stringProperty=_itoa(integerProperty, temp, 10);
			}
		}
	} break;
	}
*/
}

BEGIN_MESSAGE_MAP(StringPropertyDialog, PropertyDialog)
	//{{AFX_MSG_MAP(StringPropertyDialog)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// StringPropertyDialog message handlers

void StringPropertyDialog::OnSize(UINT nType, int cx, int cy) 
{
	PropertyDialog::OnSize(nType, cx, cy);
	_positionControls();
}

BOOL StringPropertyDialog::OnInitDialog() 
{
	PropertyDialog::OnInitDialog();
	_positionControls();
	return TRUE;
}
