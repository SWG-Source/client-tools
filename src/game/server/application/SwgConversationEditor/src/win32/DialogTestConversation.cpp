// ======================================================================
//
// DialogTestConversation.cpp
// asommers 2003-09-23
//
// copyright2003, sony online entertainment
// 
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "DialogTestConversation.h"

#include "Conversation.h"
#include "DialogChooseBranch.h"

// ======================================================================

namespace DialogTestConversationNamespace
{
	int const c_maximumNumberOfBranches = 5;

	bool isBranch (ConversationItem const * const conversationItem)
	{
		return dynamic_cast<ConversationBranch const *> (conversationItem) != 0;
	}

	bool isResponse (ConversationItem const * const conversationItem)
	{
		return dynamic_cast<ConversationResponse const *> (conversationItem) != 0;
	}
};

using namespace DialogTestConversationNamespace;

// ======================================================================

DialogTestConversation::DialogTestConversation(Conversation const * const conversation, ConversationItem const * const conversationItem, CWnd * const wnd) : 
	CDialog (DialogTestConversation::IDD, wnd),
	m_conversation (conversation),
	m_conversationItem (conversationItem)
{
	//{{AFX_DATA_INIT(DialogTestConversation)
	m_text = _T("");
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogTestConversation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogTestConversation)
	DDX_Control(pDX, IDC_BUTTON4, m_button4);
	DDX_Control(pDX, IDC_BUTTON3, m_button3);
	DDX_Control(pDX, IDC_BUTTON2, m_button2);
	DDX_Control(pDX, IDC_BUTTON1, m_button1);
	DDX_Control(pDX, IDC_BUTTON0, m_button0);
	DDX_Text(pDX, IDC_TEXT, m_text);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogTestConversation, CDialog)
	//{{AFX_MSG_MAP(DialogTestConversation)
	ON_BN_CLICKED(IDC_BUTTON0, OnButton0)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

void DialogTestConversation::update ()
{
	if (m_conversationItem)
	{
		ConversationBranch const * branch = 0;
		if (isResponse (m_conversationItem))
		{
			ConversationResponse const * const response = safe_cast<ConversationResponse const *> (m_conversationItem);
			ConversationResponse const * const linkedResponse = response->getLinkFamilyId () ? safe_cast<ConversationResponse const *> (m_conversation->getLink (response->getLinkFamilyId ())) : response;

			if (linkedResponse->getNumberOfBranches () > 1)
			{
				DialogChooseBranch dlg (m_conversation, linkedResponse, this);
				dlg.DoModal ();

				branch = linkedResponse->getBranch (dlg.m_result);
			}
			else
				branch = linkedResponse->getBranch (0);

			m_conversationItem = branch;
		}
		else
			branch = safe_cast<ConversationBranch const *> (m_conversationItem);

		std::string textboxtext = branch->getText ();
		if(branch->getGroupEcho ())
		{
			textboxtext += " [Group Echo]";
		}

		m_text = textboxtext.c_str ();

		ConversationBranch const * const linkedBranch = branch->getLinkFamilyId () ? safe_cast<ConversationBranch const *> (m_conversation->getLink (branch->getLinkFamilyId ())) : branch;
		if (linkedBranch->getNumberOfResponses () > c_maximumNumberOfBranches)
		{
			CString message;
			message.Format ("asommers didn't think you'd need more than %i responses, so only %i are shown.  Please see asommers if you'd like this changed.", c_maximumNumberOfBranches, c_maximumNumberOfBranches);
			MessageBox (message, "Not all conversation responses are shown!", MB_OK);
		}

		CButton * buttonList [c_maximumNumberOfBranches];
		buttonList [0] = &m_button0;
		buttonList [1] = &m_button1;
		buttonList [2] = &m_button2;
		buttonList [3] = &m_button3;
		buttonList [4] = &m_button4;

		for (int i = 0; i < c_maximumNumberOfBranches; ++i)
		{
			if (i < linkedBranch->getNumberOfResponses ())
			{
				std::string buttontext = linkedBranch->getResponse (i)->getText ();
				if(linkedBranch->getGroupEcho ())
				{
					buttontext += " [Group Echo]";
				}

				buttonList [i]->ShowWindow (SW_SHOW);
				buttonList [i]->EnableWindow (true);
				buttonList [i]->SetWindowText (buttontext.c_str ());
			}
			else
			{
				buttonList [i]->ShowWindow (SW_HIDE);
				buttonList [i]->EnableWindow (false);
			}
		}
	}

	UpdateData (false);
}

// ----------------------------------------------------------------------

BOOL DialogTestConversation::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	update ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------

void DialogTestConversation::processButton (int const buttonIndex)
{
	ConversationBranch const * const branch = safe_cast<ConversationBranch const *> (m_conversationItem);
	ConversationBranch const * const linkedBranch = branch->getLinkFamilyId () ? safe_cast<ConversationBranch const *> (m_conversation->getLink (branch->getLinkFamilyId ())) : branch;
	if (linkedBranch->getNumberOfResponses ())
	{
		m_conversationItem = linkedBranch->getResponse (buttonIndex);
		update ();
	}
}

// ----------------------------------------------------------------------

void DialogTestConversation::OnButton0() 
{
	processButton (0);
}

// ----------------------------------------------------------------------

void DialogTestConversation::OnButton1() 
{
	processButton (1);
}

// ----------------------------------------------------------------------

void DialogTestConversation::OnButton2() 
{
	processButton (2);
}

// ----------------------------------------------------------------------

void DialogTestConversation::OnButton3() 
{
	processButton (3);
}

// ----------------------------------------------------------------------

void DialogTestConversation::OnButton4() 
{
	processButton (4);
}

// ======================================================================

