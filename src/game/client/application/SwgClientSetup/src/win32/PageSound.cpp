// ======================================================================
//
// PageSound.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "PageSound.h"

#include "ClientMachine.h"
#include "Options.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================

IMPLEMENT_DYNCREATE(PageSound, CPropertyPage)

PageSound::PageSound() : CPropertyPage(PageSound::IDD)
{
	//{{AFX_DATA_INIT(PageSound)
	m_disableAudio = FALSE;
	m_soundVersion = _T("");

	m_lblDisableAudio = _T("");
	m_lblMilesVersion = _T("");
	m_lblMilesProvider = _T("");

	//}}AFX_DATA_INIT
}

PageSound::~PageSound()
{
}

void PageSound::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PageSound)
	DDX_Control(pDX, IDC_CHECK_DISABLEAUDIO, m_disableAudioButton);
	DDX_Control(pDX, IDC_SOUNDPROVIDER, m_soundProvider);
	DDX_Check(pDX, IDC_CHECK_DISABLEAUDIO, m_disableAudio);
	DDX_Text(pDX, IDC_SOUNDVERSION, m_soundVersion);

	DDX_Text(pDX, IDC_CHECK_DISABLEAUDIO, m_lblDisableAudio);
	DDX_Text(pDX, IDC_LBL_SOUND_MILES_VERSION, m_lblMilesVersion);
	DDX_Text(pDX, IDC_LBL_SOUND_MILES_PROVIDER, m_lblMilesProvider);

	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(PageSound, CPropertyPage)
	//{{AFX_MSG_MAP(PageSound)
	ON_BN_CLICKED(IDC_CHECK_DISABLEAUDIO, OnCheckDisableaudio)
	ON_CBN_SELCHANGE(IDC_SOUNDPROVIDER, OnSelchangeSoundprovider)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

void PageSound::initializeDialog() 
{

	VERIFY(m_lblDisableAudio.LoadString(IDS_SOUND_DISABLE_AUDIO));
	VERIFY(m_lblMilesVersion.LoadString(IDS_SOUND_MILES_VERSION));
	VERIFY(m_lblMilesProvider.LoadString(IDS_SOUND_MILES_PROVIDER));
		
	UpdateData(false);
		
}

// ======================================================================

BOOL PageSound::OnSetActive( )
{
	BOOL const result = CPropertyPage::OnSetActive();
	initializeDialog();
	return result;
}

// ======================================================================

BOOL PageSound::OnInitDialog() 
{
	BOOL const result = CPropertyPage::OnInitDialog();

	// TODO: Add extra initialization here
	m_disableAudio = ClientMachine::getNumberOfSoundProviders () > 0 ? Options::getDisableAudio () : TRUE;
	m_disableAudioButton.EnableWindow (ClientMachine::getNumberOfSoundProviders () > 0);
	m_soundProvider.EnableWindow (ClientMachine::getNumberOfSoundProviders () > 0);

	m_soundVersion = ClientMachine::getSoundVersion ();

	if (ClientMachine::getNumberOfSoundProviders () == 0)
	{
		CString providerNone;
		VERIFY(providerNone.LoadString(IDS_NONE));

		m_soundProvider.AddString (providerNone);
		m_soundProvider.SetCurSel (0);
	}
	else
	{
		for (int i = 0; i < ClientMachine::getNumberOfSoundProviders (); ++i)
			m_soundProvider.AddString (ClientMachine::getSoundProvider (i));

		int defaultSelection = 0;
		for (defaultSelection = 0; defaultSelection < ClientMachine::getNumberOfSoundProviders (); ++defaultSelection)
			if (_tcsicmp (_T("Windows Speaker Configuration"), ClientMachine::getSoundProvider (defaultSelection)) == 0)
				break;

		int selection = ClientMachine::getNumberOfSoundProviders ();
		if (Options::getSoundProvider () && *Options::getSoundProvider ())
		{
			for (selection = 0; selection < ClientMachine::getNumberOfSoundProviders (); ++selection)
				if (_tcsicmp (Options::getSoundProvider (), ClientMachine::getSoundProvider (selection)) == 0)
					break;
		}

		if (selection == ClientMachine::getNumberOfSoundProviders ())
			selection = defaultSelection;

		m_soundProvider.SetCurSel (selection);
	}

	UpdateData (false);

	initializeDialog();
	return result;
}

// ----------------------------------------------------------------------

void PageSound::applyOptions ()
{
	UpdateData (true);

	Options::setDisableAudio (m_disableAudio == TRUE);
	Options::setSoundProvider (ClientMachine::getSoundProvider (m_soundProvider.GetCurSel ()));
}

// ----------------------------------------------------------------------

void PageSound::OnCheckDisableaudio() 
{
	applyOptions ();
}

// ----------------------------------------------------------------------

void PageSound::OnSelchangeSoundprovider() 
{
	applyOptions ();
}

// ======================================================================

