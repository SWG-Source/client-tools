// ======================================================================
//
// PageGame.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "PageGame.h"

#include "ClientMachine.h"
#include "Options.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================

IMPLEMENT_DYNCREATE(PageGame, CPropertyPage)

// ----------------------------------------------------------------------

PageGame::PageGame() : CPropertyPage(PageGame::IDD)
{
	//{{AFX_DATA_INIT(PageGame)
	m_disableCharacterLodManager = FALSE;
	m_skipIntro = FALSE;

	m_lblSkipIntro = _T("");
	m_lblDisableCharacterLodManager = _T("");

	//}}AFX_DATA_INIT

	m_userChangedLanguage = false;
}

// ----------------------------------------------------------------------

PageGame::~PageGame()
{
}

// ----------------------------------------------------------------------

void PageGame::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PageGame)
	DDX_Control(pDX, IDC_CHECK_ENABLEFORCEFEEDBACK, m_enableForceFeedback);
	DDX_Control(pDX, IDC_CHECK_ENABLEJOYSTICKS, m_enableJoysticks);
	DDX_Control(pDX, IDC_GAMECONTROLLER, m_joystick);
	DDX_Control(pDX, IDC_DROP_LANGUAGE, m_language);
	DDX_Check(pDX, IDC_CHECK_DISABLECHARACTERLODMANAGER, m_disableCharacterLodManager);
	DDX_Check(pDX, IDC_CHECK_SKIPINTRO, m_skipIntro);

	DDX_Text(pDX, IDC_CHECK_SKIPINTRO, m_lblSkipIntro);
	DDX_Text(pDX, IDC_CHECK_DISABLECHARACTERLODMANAGER, m_lblDisableCharacterLodManager);
	DDX_Text(pDX, IDC_LBL_DROP_LANGUAGE, m_lblLanguage);
	DDX_Text(pDX, IDC_CHECK_ENABLEJOYSTICKS, m_lblEnableJoysticks);
	DDX_Text(pDX, IDC_STATIC_JOYSTICK, m_lblJoystick);
	DDX_Text(pDX, IDC_CHECK_ENABLEFORCEFEEDBACK, m_lblEnabledForceFeedback);
	//}}AFX_DATA_MAP
}

// ======================================================================

BEGIN_MESSAGE_MAP(PageGame, CPropertyPage)
	//{{AFX_MSG_MAP(PageGame)
	ON_BN_CLICKED(IDC_CHECK_SKIPINTRO, OnCheckSkipintro)
	ON_BN_CLICKED(IDC_CHECK_DISABLECHARACTERLODMANAGER, OnCheckDisablecharacterlodmanager)
	ON_BN_CLICKED(IDC_CHECK_ENABLEJOYSTICKS, OnCheckEnablejoysticks)
	ON_CBN_SELCHANGE(IDC_GAMECONTROLLER, OnSelchangeGamecontroller)
	ON_CBN_SELCHANGE(IDC_DROP_LANGUAGE, OnSelchangeLanguage)
	ON_BN_CLICKED(IDC_CHECK_ENABLEFORCEFEEDBACK, OnCheckEnableforcefeedback)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

void PageGame::OnCheckSkipintro() 
{
	applyOptions ();
}

// ----------------------------------------------------------------------

void PageGame::OnCheckDisablecharacterlodmanager() 
{
	applyOptions ();
}

// ----------------------------------------------------------------------

void PageGame::OnCheckEnablejoysticks() 
{
	if (ClientMachine::getNumberOfJoysticks())
	{
		m_joystick.EnableWindow(m_enableJoysticks.GetCheck());	

		if (m_enableJoysticks.GetCheck() == FALSE)
		{
			m_enableForceFeedback.EnableWindow(FALSE);
			m_enableForceFeedback.SetCheck(FALSE);
		}
		else
		{
			OnSelchangeGamecontroller();
		}
	}
	else
	{
		m_joystick.EnableWindow(FALSE);	
		m_enableForceFeedback.EnableWindow(FALSE);
		m_enableForceFeedback.SetCheck(FALSE);
	}

	applyOptions ();
}

// ----------------------------------------------------------------------

void PageGame::OnSelchangeGamecontroller() 
{
	if (ClientMachine::getJoystickSupportsForceFeedback(m_joystick.GetCurSel()))
	{
		if (!m_enableForceFeedback.IsWindowEnabled())
		{
			m_enableForceFeedback.EnableWindow(TRUE);
			m_enableForceFeedback.SetCheck(TRUE);
		}
	}
	else
	{
		m_enableForceFeedback.EnableWindow(FALSE);
		m_enableForceFeedback.SetCheck(FALSE);
	}

	applyOptions ();
}

// ----------------------------------------------------------------------

void PageGame::OnSelchangeLanguage() 
{
	m_userChangedLanguage = true;
	switch(m_language.GetCurSel())
	{
	case 0:
		m_localeCode = _T("en");
		m_fontLocaleCode = _T("en");
		break;
	case 1:
		m_localeCode = _T("ja");
		m_fontLocaleCode = _T("j5");
		break;
	default:
		m_localeCode = _T("en");
		m_fontLocaleCode = _T("en");
		break;
	}
	applyOptions ();
}

// ----------------------------------------------------------------------

void PageGame::OnCheckEnableforcefeedback() 
{
	applyOptions ();
}
// ======================================================================

void PageGame::initializeDialog() 
{

	VERIFY(m_lblSkipIntro.LoadString(IDS_GAME_SKIP_INTRO));
	VERIFY(m_lblDisableCharacterLodManager.LoadString(IDS_GAME_DISABLE_LOD));
	VERIFY(m_lblLanguage.LoadString(IDS_LANGUAGE));
	VERIFY(m_lblEnableJoysticks.LoadString(IDS_ENABLE_JOYSTICKS));
	VERIFY(m_lblJoystick.LoadString(IDS_JOYSTICK));
	VERIFY(m_lblEnabledForceFeedback.LoadString(IDS_ENABLE_FORCE_FEEDBACK));

	UpdateData (false);	
}

// ======================================================================

BOOL PageGame::OnSetActive( )
{
	BOOL const result = CPropertyPage::OnSetActive();
	initializeDialog();
	return result;
}

// ======================================================================

BOOL PageGame::OnInitDialog() 
{
	BOOL const result = CPropertyPage::OnInitDialog();

	m_disableCharacterLodManager = Options::getDisableCharacterLodManager ();
	m_skipIntro = Options::getSkipIntro ();
	int const numberOfJoysticks = ClientMachine::getNumberOfJoysticks();
	if (numberOfJoysticks)
	{
		for (int i = 0; i < numberOfJoysticks; ++i)
			m_joystick.AddString (ClientMachine::getJoystickDescription(i));

		m_enableJoysticks.SetCheck(Options::getUseJoysticks());
		m_joystick.SetCurSel(Options::getUseJoystick());

		if (ClientMachine::getJoystickSupportsForceFeedback(Options::getUseJoystick()))
		{
			m_enableForceFeedback.SetCheck(Options::getUseForceFeedback());
		}
		else
		{
			m_enableForceFeedback.EnableWindow(FALSE);
			m_enableForceFeedback.SetCheck(FALSE);
		}
	}
	else
	{
		m_enableJoysticks.EnableWindow(FALSE);		
		m_joystick.EnableWindow(FALSE);		

		m_enableForceFeedback.EnableWindow(FALSE);
		m_enableForceFeedback.SetCheck(FALSE);
	}

	m_language.AddString(_T("English"));
	m_language.AddString(_T("Japanese"));

	if (wcsncmp(Options::getLocaleCode(), _T("ja"), 2) == 0)
	{
		m_language.SetCurSel(1);
		m_localeCode = _T("ja");
		m_fontLocaleCode = _T("j5");
	}
	else
	{
		m_localeCode = _T("en");
		m_fontLocaleCode = _T("en");
		m_language.SetCurSel(0);
	}

	UpdateData (false);
	
	initializeDialog();
	return result;
}

// ----------------------------------------------------------------------

void PageGame::applyOptions ()
{
	UpdateData (true);

	Options::setDisableCharacterLodManager (m_disableCharacterLodManager == TRUE);
	Options::setSkipIntro (m_skipIntro == TRUE);
	Options::setUseJoysticks(m_enableJoysticks.GetCheck() ? true : false);
	Options::setUseForceFeedback(m_enableForceFeedback.GetCheck() ? true : false);
	Options::setUseJoystick(m_joystick.GetCurSel());
	if(m_userChangedLanguage)
	{	
		Options::setLocaleCode(m_localeCode);
		Options::setFontLocaleCode(m_fontLocaleCode);
	}
}

// ======================================================================

