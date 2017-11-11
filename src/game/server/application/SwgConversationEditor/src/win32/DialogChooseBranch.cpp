// ======================================================================
//
// DialogChooseBranch.cpp
// asommers
//
// copyright 2003, sony online entertainment
// 
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "DialogChooseBranch.h"

#include "Conversation.h"
#include "ScriptGroup.h"

// ======================================================================

namespace DialogChooseBranchNamespace
{
	int const c_maximumNumberOfBranches = 5;
};

using namespace DialogChooseBranchNamespace;

// ======================================================================

DialogChooseBranch::DialogChooseBranch (Conversation const * conversation, ConversationResponse const * response, CWnd * const wnd) : 
	CDialog(DialogChooseBranch::IDD, wnd),
	m_result (0),
	m_conversation (conversation),
	m_response (response)
{
	//{{AFX_DATA_INIT(DialogChooseBranch)
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogChooseBranch::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogChooseBranch)
	DDX_Control(pDX, IDC_BUTTON4, m_button4);
	DDX_Control(pDX, IDC_BUTTON3, m_button3);
	DDX_Control(pDX, IDC_BUTTON2, m_button2);
	DDX_Control(pDX, IDC_BUTTON1, m_button1);
	DDX_Control(pDX, IDC_BUTTON0, m_button0);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogChooseBranch, CDialog)
	//{{AFX_MSG_MAP(DialogChooseBranch)
	ON_BN_CLICKED(IDC_BUTTON0, OnButton0)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

BOOL DialogChooseBranch::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if (m_response->getNumberOfBranches () > c_maximumNumberOfBranches)
	{
		CString message;
		message.Format ("asommers didn't think you'd need more than %i branches, so only %i are shown.  Please see asommers if you'd like this changed.", c_maximumNumberOfBranches, c_maximumNumberOfBranches);
		MessageBox (message, "Not all conversation branches are shown!", MB_OK);
	}

	CButton * buttonList [c_maximumNumberOfBranches];
	buttonList [0] = &m_button0;
	buttonList [1] = &m_button1;
	buttonList [2] = &m_button2;
	buttonList [3] = &m_button3;
	buttonList [4] = &m_button4;

	for (int i = 0; i < 5; ++i)
	{
		if (i < m_response->getNumberOfBranches ())
		{
			buttonList [i]->ShowWindow (SW_SHOW);
			buttonList [i]->EnableWindow (true);

			CString text;

			text += "CONDITION: ";
			if (m_response->getBranch (i)->getNegateCondition ())
				text += "!";
			text += m_conversation->getConditionGroup ()->getFamilyName (m_response->getBranch (i)->getConditionFamilyId ());
			text += "\r\n";

			text += "TEXT: ";
			text += m_response->getBranch (i)->getText ().c_str ();

			buttonList [i]->SetWindowText (text);
		}
		else
		{
			buttonList [i]->ShowWindow (SW_HIDE);
			buttonList [i]->EnableWindow (false);
		}
	}
	
	return TRUE;
}

// ======================================================================

void DialogChooseBranch::OnButton0() 
{
	m_result = 0;

	CDialog::OnOK ();
}

// ----------------------------------------------------------------------

void DialogChooseBranch::OnButton1() 
{
	m_result = 1;

	CDialog::OnOK ();
}

// ----------------------------------------------------------------------

void DialogChooseBranch::OnButton2() 
{
	m_result = 2;

	CDialog::OnOK ();
}

// ----------------------------------------------------------------------

void DialogChooseBranch::OnButton3() 
{
	m_result = 3;

	CDialog::OnOK ();
}

// ----------------------------------------------------------------------

void DialogChooseBranch::OnButton4() 
{
	m_result = 4;

	CDialog::OnOK ();
}

// ======================================================================

