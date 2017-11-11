// BasePropertiesPage.cpp : implementation file
//

#include "FirstUiBuilder.h"
#include "BasePropertiesPage.h"


/////////////////////////////////////////////////////////////////////////////
// BasePropertiesPage property page

IMPLEMENT_DYNCREATE(BasePropertiesPage, CPropertyPage)

BasePropertiesPage::BasePropertiesPage() : CPropertyPage(BasePropertiesPage::IDD)
{
	//{{AFX_DATA_INIT(BasePropertiesPage)
	m_name = _T("");
	//}}AFX_DATA_INIT
}

BasePropertiesPage::~BasePropertiesPage()
{
}

void BasePropertiesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(BasePropertiesPage)
	DDX_Text(pDX, IDC_PROPERTY_NAME, m_name);
	DDV_MaxChars(pDX, m_name, 64);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(BasePropertiesPage, CPropertyPage)
	//{{AFX_MSG_MAP(BasePropertiesPage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BasePropertiesPage message handlers
