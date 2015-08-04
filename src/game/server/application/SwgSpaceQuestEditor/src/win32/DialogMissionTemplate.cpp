// ======================================================================
//
// DialogMissionTemplate.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceQuestEditor.h"
#include "DialogMissionTemplate.h"

#include "Configuration.h"

// ======================================================================

DialogMissionTemplate::DialogMissionTemplate(CWnd* pParent /*=NULL*/)
	: CDialog(DialogMissionTemplate::IDD, pParent)
{
	//{{AFX_DATA_INIT(DialogMissionTemplate)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogMissionTemplate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogMissionTemplate)
	DDX_Control(pDX, IDC_LIST_MISSIONTEMPLATE, m_missionTemplateListBox);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogMissionTemplate, CDialog)
	//{{AFX_MSG_MAP(DialogMissionTemplate)
	ON_LBN_DBLCLK(IDC_LIST_MISSIONTEMPLATE, OnDblclkListMissiontemplate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogMissionTemplate::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	for (int i = 0; i < Configuration::getNumberOfMissionTemplates(); ++i)
	{
		Configuration::MissionTemplate const * const missionTemplate = Configuration::getMissionTemplate(i);
		m_missionTemplateListBox.AddString(missionTemplate->getName());
		
		if (i == 0)
			m_missionTemplateListBox.SetCurSel(0);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogMissionTemplate::OnOK()
{
	UpdateData(false);

	int const index = m_missionTemplateListBox.GetCurSel();
	if (index != LB_ERR)
		m_missionTemplateListBox.GetText(index, m_result);

	CDialog::OnOK();
}

// ----------------------------------------------------------------------

CString const & DialogMissionTemplate::getResult() const
{
	return m_result;
}

// ----------------------------------------------------------------------

void DialogMissionTemplate::OnDblclkListMissiontemplate() 
{
	OnOK();
}

// ======================================================================

