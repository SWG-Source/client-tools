// ======================================================================
//
// DialogStringIdList.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceQuestEditor.h"
#include "DialogStringIdList.h"

#include "Configuration.h"

// ======================================================================

DialogStringIdList::DialogStringIdList(CString const & missionTemplateType, StringList const & stringIdList)
	: CDialog(DialogStringIdList::IDD, 0),
	m_missionTemplateType(missionTemplateType),
	m_stringIdList(stringIdList)
{
	//{{AFX_DATA_INIT(DialogStringIdList)
	m_stringId = _T("");
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogStringIdList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogStringIdList)
	DDX_Control(pDX, IDC_LIST_STRINGID, m_stringIdListBox);
	DDX_Text(pDX, IDC_EDIT_STRINGID, m_stringId);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogStringIdList, CDialog)
	//{{AFX_MSG_MAP(DialogStringIdList)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_REMOVEALL, OnButtonRemoveall)
	ON_LBN_SELCHANGE(IDC_LIST_STRINGID, OnSelchangeListStringid)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE, OnButtonChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

BOOL DialogStringIdList::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	for (StringList::iterator iter = m_stringIdList.begin(); iter != m_stringIdList.end(); ++iter)
		m_stringIdListBox.AddString(*iter);
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

StringList const & DialogStringIdList::getStringIdList() const
{
	return m_stringIdList;
}

// ----------------------------------------------------------------------

void DialogStringIdList::OnButtonAdd() 
{
	UpdateData(true);

	if (!m_stringId.IsEmpty() && std::find(m_stringIdList.begin(), m_stringIdList.end(), m_stringId) == m_stringIdList.end())
	{
		//-- See if it is a mission template string id
		Configuration::MissionTemplate const * const missionTemplate = Configuration::getMissionTemplate(m_missionTemplateType);
		if (missionTemplate->getStringTemplate(m_stringId))
		{
			CString const message("String Id " + m_stringId + " is a mission template string id");
			MessageBox(message);
		}
		else
		{
			//-- See if it is a quest string suffix
			if (missionTemplate->isQuestStringSuffixTemplate(m_stringId))
			{
				CString const message("String Id " + m_stringId + " is a quest string suffix");
				MessageBox(message);
			}
			else
			{
				int const index = m_stringIdListBox.InsertString(m_stringIdListBox.GetCount(), m_stringId);
				m_stringIdListBox.SetCurSel(index);
			}
		}
	}
}

// ----------------------------------------------------------------------

void DialogStringIdList::OnButtonRemove() 
{
	int const selection = m_stringIdListBox.GetCurSel();
	if (selection != LB_ERR)
	{
		m_stringIdListBox.DeleteString(selection);
		m_stringIdListBox.SetCurSel(selection == m_stringIdListBox.GetCount() ? selection - 1 : selection);
	}
}

// ----------------------------------------------------------------------

void DialogStringIdList::OnButtonRemoveall() 
{
	if (MessageBox("Are you sure you wish to remove all user String Id entries?", "Remove All", MB_YESNO) == IDYES)
		m_stringIdListBox.ResetContent();
}

// ----------------------------------------------------------------------

void DialogStringIdList::OnOK()
{
	m_stringIdList.clear();

	for (int i = 0; i < m_stringIdListBox.GetCount(); ++i)
	{
		CString text;
		m_stringIdListBox.GetText(i, text);

		m_stringIdList.push_back(text);
	}

	CDialog::OnOK();
}

// ----------------------------------------------------------------------

void DialogStringIdList::OnButtonChange() 
{
	UpdateData(true);

	if (!m_stringId.IsEmpty() && std::find(m_stringIdList.begin(), m_stringIdList.end(), m_stringId) == m_stringIdList.end())
	{
		Configuration::MissionTemplate const * const missionTemplate = Configuration::getMissionTemplate(m_missionTemplateType);
		if (missionTemplate->getStringTemplate(m_stringId))
		{
			CString const message("String Id " + m_stringId + " is a mission template String Id");
			MessageBox(message);
		}
		else
		{
			int const selection = m_stringIdListBox.GetCurSel();
			if (selection != LB_ERR)
			{
				m_stringIdListBox.DeleteString(selection);
				m_stringIdListBox.InsertString(selection, m_stringId);
				m_stringIdListBox.SetCurSel(selection);
			}
		}
	}
}

// ----------------------------------------------------------------------

void DialogStringIdList::OnSelchangeListStringid() 
{
	int const selection = m_stringIdListBox.GetCurSel();
	if (selection != LB_ERR)
	{
		m_stringIdListBox.GetText(selection, m_stringId);
		UpdateData(false);
	}
}

// ======================================================================
