// ======================================================================
//
// DialogPoll.cpp
// rsitton
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "DialogPoll.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DialogPoll dialog

namespace DialogPollNamespace
{
	void updateButtonSize(CButton & button, CButton * previusButton = NULL);
}

// ----------------------------------------------------------------------

void DialogPollNamespace::updateButtonSize(CButton & button, CButton * previusButton)
{
	CString szText;
	button.GetWindowText(szText);
	
	CDC const * const dc = button.GetWindowDC();
	if (dc) 
	{
		TEXTMETRIC tm;
		dc->GetTextMetrics(&tm);
		
		CSize const & textSize = dc->GetTextExtent(szText);

		// Get the current rect.
		CRect currentRectScreen;
		button.GetWindowRect(currentRectScreen);
		
		// Get new width.
		int const spacing = (tm.tmAveCharWidth + tm.tmMaxCharWidth) / 2;
		int const width = max(textSize.cx + spacing, currentRectScreen.Width());
		int const height = max(textSize.cy * 2, currentRectScreen.Height());
		
		int updateFlags = SWP_NOZORDER | SWP_SHOWWINDOW;
		
		POINT buttonPosition;
		
		if (previusButton) 
		{
			// Update the position of the object.
			CRect prevRectScreen;
			previusButton->GetWindowRect(prevRectScreen);

			buttonPosition.x = prevRectScreen.left + prevRectScreen.Width() + spacing;
			buttonPosition.y = currentRectScreen.top;
			
			button.GetParent()->ScreenToClient(&buttonPosition);
		}
		else
		{
			// Don't move the button.
			updateFlags |= SWP_NOMOVE;
		}
		
		button.SetWindowPos(NULL, buttonPosition.x, buttonPosition.y, width, height, updateFlags);
	}
}

using namespace DialogPollNamespace;

// ----------------------------------------------------------------------

IMPLEMENT_DYNAMIC(DialogPoll, CDialog);

DialogPoll::DialogPoll(CWnd* pParent /*=NULL*/)
	: CDialog(DialogPoll::IDD, pParent), m_buttons(), m_buttonsActive(0), m_buttonSelectedText()
{
	//{{AFX_DATA_INIT(DialogPoll)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

void DialogPoll::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogPoll)
	DDX_Control(pDX, IDC_BUTTON_TEMPLATE7, m_buttonTemplate6);
	DDX_Control(pDX, IDC_BUTTON_TEMPLATE6, m_buttonTemplate5);
	DDX_Control(pDX, IDC_BUTTON_TEMPLATE5, m_buttonTemplate4);
	DDX_Control(pDX, IDC_STATIC_INPUT_DESC, m_inputDesc);
	DDX_Control(pDX, IDC_BUTTON_TEMPLATE, m_buttonTemplate0);
	DDX_Control(pDX, IDC_BUTTON_TEMPLATE2, m_buttonTemplate1);
	DDX_Control(pDX, IDC_BUTTON_TEMPLATE3, m_buttonTemplate2);
	DDX_Control(pDX, IDC_BUTTON_TEMPLATE4, m_buttonTemplate3);
	DDX_Control(pDX, IDC_STATIC_QUESTION_TEXT, m_questionText);
	DDX_Control(pDX, IDC_EDIT_TEMPLATE, m_templateText);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogPoll, CDialog)
	//{{AFX_MSG_MAP(DialogPoll)
	ON_BN_CLICKED(IDC_BUTTON_TEMPLATE, OnButtonTemplate0)
	ON_BN_CLICKED(IDC_BUTTON_TEMPLATE2, OnButtonTemplate1)
	ON_BN_CLICKED(IDC_BUTTON_TEMPLATE3, OnButtonTemplate2)
	ON_BN_CLICKED(IDC_BUTTON_TEMPLATE4, OnButtonTemplate3)
	ON_BN_CLICKED(IDC_BUTTON_TEMPLATE5, OnButtonTemplate4)
	ON_BN_CLICKED(IDC_BUTTON_TEMPLATE6, OnButtonTemplate5)
	ON_BN_CLICKED(IDC_BUTTON_TEMPLATE7, OnButtonTemplate6)
	ON_WM_SHOWWINDOW()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DialogPoll message handlers

// ----------------------------------------------------------------------

void DialogPoll::addButton(CString const & name, bool isDefault)
{
	ASSERT(m_buttonsActive < m_buttons.size());

	m_buttonsActive = clamp(0, m_buttonsActive, static_cast<int>(m_buttons.size() - 1));

	CButton * const newButton = m_buttons[m_buttonsActive];

	newButton->EnableWindow();
	newButton->ShowWindow(SW_SHOW);			
	newButton->SetWindowText(name);

	m_buttonsActive++;

	// Re-align buttons here.
	updateButtonPositions();
}

// ----------------------------------------------------------------------

void DialogPoll::deleteButtons()
{
	m_buttons.clear();
}

// ----------------------------------------------------------------------

void DialogPoll::setQuestionText(CString const & questionText)
{
	m_questionText.SetWindowText(questionText);
	BOOL const enableWindow = !questionText.IsEmpty();
	m_questionText.EnableWindow(enableWindow);
	m_questionText.ShowWindow(enableWindow ? SW_SHOW : SW_HIDE);
}

// ----------------------------------------------------------------------

BOOL DialogPoll::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Setup the buttons. (Add new buttons here)
	m_buttons.push_back(&m_buttonTemplate0);
	m_buttons.push_back(&m_buttonTemplate1);
	m_buttons.push_back(&m_buttonTemplate2);
	m_buttons.push_back(&m_buttonTemplate3);
	m_buttons.push_back(&m_buttonTemplate4);
	m_buttons.push_back(&m_buttonTemplate5);
	m_buttons.push_back(&m_buttonTemplate6);

	for (Buttons::iterator itButton = m_buttons.begin(); itButton != m_buttons.end(); ++itButton) 
	{
		CButton * const button = *itButton;
		button->ShowWindow(SW_HIDE);
		button->EnableWindow(FALSE);
	}

	// Setup the edit text.
	m_templateText.ShowWindow(SW_HIDE);
	m_templateText.EnableWindow(FALSE);

	m_inputDesc.ShowWindow(SW_HIDE);
	m_inputDesc.EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------
// TODO: Move into Cmd message.
void DialogPoll::OnButtonTemplate0() 
{
	m_buttonTemplate0.GetWindowText(m_buttonSelectedText);
	setButtonFocus();
}

// ----------------------------------------------------------------------

void DialogPoll::OnButtonTemplate1() 
{
	m_buttonTemplate1.GetWindowText(m_buttonSelectedText);
	setButtonFocus();
}

// ----------------------------------------------------------------------

void DialogPoll::OnButtonTemplate2() 
{
	m_buttonTemplate2.GetWindowText(m_buttonSelectedText);
	setButtonFocus();
}

// ----------------------------------------------------------------------

void DialogPoll::OnButtonTemplate3() 
{
	m_buttonTemplate3.GetWindowText(m_buttonSelectedText);
	setButtonFocus();
}

// ----------------------------------------------------------------------

void DialogPoll::OnButtonTemplate4() 
{
	m_buttonTemplate4.GetWindowText(m_buttonSelectedText);
	setButtonFocus();
}

// ----------------------------------------------------------------------

void DialogPoll::OnButtonTemplate5() 
{
	m_buttonTemplate5.GetWindowText(m_buttonSelectedText);
	setButtonFocus();
}

// ----------------------------------------------------------------------

void DialogPoll::OnButtonTemplate6() 
{
	m_buttonTemplate6.GetWindowText(m_buttonSelectedText);
	setButtonFocus();
}

// ----------------------------------------------------------------------

CString DialogPoll::getSelectedButtonText() const
{
	return m_buttonSelectedText;
}

// ----------------------------------------------------------------------

CString DialogPoll::getInputText() const
{
	CString questionText;
	m_questionText.GetWindowText(questionText);
	return questionText;
}

// ----------------------------------------------------------------------

void DialogPoll::setInputText(CString const & description, bool enable)
{
	BOOL enableWindow = enable ? TRUE : FALSE;
	m_templateText.EnableWindow(enableWindow);
	m_templateText.ShowWindow(enableWindow ? SW_SHOW : SW_HIDE);

	m_inputDesc.SetWindowText(description);
	m_inputDesc.EnableWindow(enableWindow);
	m_inputDesc.ShowWindow(enableWindow ? SW_SHOW : SW_HIDE);
}

// ----------------------------------------------------------------------

void DialogPoll::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	setButtonFocus();
}

// ----------------------------------------------------------------------

void DialogPoll::setButtonFocus()
{
	for (Buttons::iterator itButton = m_buttons.begin(); itButton != m_buttons.end(); ++itButton) 
	{
		CButton * const button = *itButton;
		
		CString buttonText;
		button->GetWindowText(buttonText);

		bool const isSelected = button->IsWindowEnabled() && (buttonText == m_buttonSelectedText);
		button->SetState(isSelected);
	}
}

// ----------------------------------------------------------------------

void DialogPoll::updateButtonPositions()
{
	CButton * previousButton = NULL;

	for (Buttons::iterator itButton = m_buttons.begin(); itButton != m_buttons.end(); ++itButton) 
	{
		CButton * const button = *itButton;
		
		BOOL const isEnabled = button->IsWindowEnabled();
		if (isEnabled) 
		{
			updateButtonSize(*button, previousButton);
			previousButton = button;
		}
	}
}

// ----------------------------------------------------------------------

void DialogPoll::getMessageInformation(CString & msgInfo)
{
	// Dump the question name.
	CString questionName;
	GetWindowText(questionName);
	msgInfo += _T("\"");
	msgInfo += questionName;
	msgInfo += _T("\",");

	// Dump the selected button.
	msgInfo += _T("\"");
	msgInfo += m_buttonSelectedText;
	msgInfo += _T("\",");

	// Dump user input.
	CString questionText;
	m_templateText.GetWindowText(questionText);
	trimString(questionText);

	msgInfo += _T("\"");
	msgInfo += questionText;
	msgInfo += _T("\"");
}


// ----------------------------------------------------------------------

void DialogPoll::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	setButtonFocus();
}
