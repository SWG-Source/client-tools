// ======================================================================
//
// PageAdvanced.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "PageAdvanced.h"

#include "ClientMachine.h"
#include "Options.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================

static int DetermineMemoryManagerSize ()
{
	MEMORYSTATUS memoryStatus;
	GlobalMemoryStatus (&memoryStatus);

	//-- memory status returns bytes
	const int numberOfMegabytes = (memoryStatus.dwTotalPhys / 4) * 3;

	if (numberOfMegabytes > 2000 * 1024 * 1024)
		return 2000 * 1024 * 1024;

	if (numberOfMegabytes < 250 * 1024 * 1024)
		return 250 * 1024 * 1024;

	return numberOfMegabytes;
}

// ======================================================================

IMPLEMENT_DYNCREATE(PageAdvanced, CPropertyPage)

PageAdvanced::PageAdvanced() : CPropertyPage(PageAdvanced::IDD)
{
	//{{AFX_DATA_INIT(PageAdvanced)
	m_disableWorldPreloading = FALSE;
	m_skipL0Characters = FALSE;
	m_skipL0Meshes = FALSE;
	m_gameMemorySize = _T("");
	m_disableTextureBaking = FALSE;
	m_disableFileCaching = FALSE;
	m_disableAsynchronousLoader = FALSE;

	m_lblHeader = _T("");
	m_lblDisableWordPreloading = _T("");
	m_lblUseLowDetailCharacters = _T("");
	m_lblUseLowDetailMeshes = _T("");
	m_lblDisableTextureBaking = _T("");
	m_lblDisableFileCaching = _T("");
	m_lblDisableAsynchronousLoader = _T("");
	m_lblGameMemorySize = _T("");
	m_lblGameMemoryInfo = _T("");

	//}}AFX_DATA_INIT
}

PageAdvanced::~PageAdvanced()
{
}

void PageAdvanced::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PageAdvanced)
	DDX_Control(pDX, IDC_CHECK_SKIPL0MESHES, m_skipL0MeshesButton);
	DDX_Check(pDX, IDC_CHECK_DISABLEWORLDPRELOADING, m_disableWorldPreloading);
	DDX_Check(pDX, IDC_CHECK_SKIPL0CHARACTERS, m_skipL0Characters);
	DDX_Check(pDX, IDC_CHECK_SKIPL0MESHES, m_skipL0Meshes);
	DDX_Text(pDX, IDC_STATIC_GAMEMEMORY, m_gameMemorySize);
	DDX_Check(pDX, IDC_CHECK_DISABLETEXTUREBAKING, m_disableTextureBaking);
	DDX_Check(pDX, IDC_CHECK_DISABLEFILECACHING, m_disableFileCaching);
	DDX_Check(pDX, IDC_CHECK_DISABLEASYNCHRONOUSLOADER, m_disableAsynchronousLoader);

	DDX_Text(pDX, IDC_LBL_ADVANCED_HEADER, m_lblHeader);
	DDX_Text(pDX, IDC_CHECK_DISABLEWORLDPRELOADING, m_lblDisableWordPreloading);
	DDX_Text(pDX, IDC_CHECK_SKIPL0CHARACTERS, m_lblUseLowDetailCharacters);
	DDX_Text(pDX, IDC_CHECK_SKIPL0MESHES, m_lblUseLowDetailMeshes);
	DDX_Text(pDX, IDC_CHECK_DISABLETEXTUREBAKING, m_lblDisableTextureBaking);
	DDX_Text(pDX, IDC_CHECK_DISABLEFILECACHING, m_lblDisableFileCaching);
	DDX_Text(pDX, IDC_CHECK_DISABLEASYNCHRONOUSLOADER, m_lblDisableAsynchronousLoader);
	DDX_Text(pDX, IDC_LBL_ADVANCED_GAME_MEMORY_SIZE, m_lblGameMemorySize);
	DDX_Text(pDX, IDC_LBL_ADVANCED_GAME_MEMORY_INFO, m_lblGameMemoryInfo);

	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(PageAdvanced, CPropertyPage)
	//{{AFX_MSG_MAP(PageAdvanced)
	ON_BN_CLICKED(IDC_CHECK_DISABLESHADOWS, OnCheckDisableshadows)
	ON_BN_CLICKED(IDC_CHECK_DISABLEWORLDPRELOADING, OnCheckDisableworldpreloading)
	ON_BN_CLICKED(IDC_CHECK_SKIPL0CHARACTERS, OnCheckSkipl0characters)
	ON_BN_CLICKED(IDC_CHECK_SKIPL0MESHES, OnCheckSkipl0meshes)
	ON_BN_CLICKED(IDC_CHECK_DISABLETEXTUREBAKING, OnCheckDisabletexturebaking)
	ON_BN_CLICKED(IDC_CHECK_DISABLEFILECACHING, OnCheckDisablefilecaching)
	ON_BN_CLICKED(IDC_CHECK_DISABLEASYNCHRONOUSLOADER, OnCheckDisableasynchronousloader)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

void PageAdvanced::initializeDialog() 
{
	VERIFY(m_lblHeader.LoadString(IDS_ADVANCED_HEADER));
	VERIFY(m_lblDisableWordPreloading.LoadString(IDS_ADVANCED_DISABLE_WORLD_PRELOADING));
	VERIFY(m_lblUseLowDetailCharacters.LoadString(IDS_ADVANCED_USE_LOW_DETAIL_CHARACTERS));
	VERIFY(m_lblUseLowDetailMeshes.LoadString(IDS_ADVANCED_USE_LOW_DETAIL_MESHES));
	VERIFY(m_lblDisableTextureBaking.LoadString(IDS_ADVANCED_DISABLE_TEXTURE_BAKING));
	VERIFY(m_lblDisableFileCaching.LoadString(IDS_ADVANCED_DISABLE_FILE_CACHING));
	VERIFY(m_lblDisableAsynchronousLoader.LoadString(IDS_ADVANCED_DISABLE_ASYNCHRONOUS_LOADER));
	VERIFY(m_lblGameMemorySize.LoadString(IDS_ADVANCED_GAME_MEMORY_SIZE));
	VERIFY(m_lblGameMemoryInfo.LoadString(IDS_ADVANCED_GAME_MEMORY_INFO));
	
	// TODO: Add extra initialization here
	m_disableWorldPreloading = Options::getDisableWorldPreloading ();
	m_skipL0Characters = Options::getSkipL0Characters ();
	m_skipL0Meshes = Options::getSkipL0Meshes ();
	m_disableTextureBaking = Options::getDisableTextureBaking ();
	m_disableFileCaching = Options::getDisableFileCaching ();
	m_disableAsynchronousLoader = Options::getDisableAsynchronousLoader ();

	const int size = DetermineMemoryManagerSize ();
	m_gameMemorySize.Format (_T("%i MB"), size / (1024 * 1024));

	if (ClientMachine::getPhysicalMemorySize() < 260)
	{
		//Set low detail meshes to true, and disable that control
		Options::setSkipL0Meshes(true);
		m_skipL0Meshes = true;
		m_skipL0MeshesButton.EnableWindow(false);
	}
	else if (ClientMachine::getPhysicalMemorySize() < 400)
	{
		//Set low detail meshes to true
		Options::setSkipL0Meshes(true);
		m_skipL0Meshes = true;		
	}

	UpdateData (false);
}

// ======================================================================

BOOL PageAdvanced::OnSetActive()
{
	BOOL const result = CPropertyPage::OnSetActive();
	initializeDialog();
	return result;
}

// ======================================================================

BOOL PageAdvanced::OnInitDialog() 
{
	BOOL const result = CPropertyPage::OnInitDialog();
	initializeDialog();
	return result;
}

// ----------------------------------------------------------------------

void PageAdvanced::applyOptions ()
{
	UpdateData (true);

	Options::setDisableWorldPreloading (m_disableWorldPreloading == TRUE);
	Options::setSkipL0Characters (m_skipL0Characters == TRUE);
	Options::setSkipL0Meshes (m_skipL0Meshes == TRUE);
	Options::setDisableTextureBaking (m_disableTextureBaking == TRUE);
	Options::setDisableFileCaching (m_disableFileCaching == TRUE);
	Options::setDisableAsynchronousLoader (m_disableAsynchronousLoader == TRUE);
}

// ----------------------------------------------------------------------

void PageAdvanced::OnCheckDisableshadows() 
{
	applyOptions ();	
}

// ----------------------------------------------------------------------

void PageAdvanced::OnCheckDisableworldpreloading() 
{
	applyOptions ();	
}

// ----------------------------------------------------------------------

void PageAdvanced::OnCheckSkipl0characters() 
{
	applyOptions ();	
}

// ----------------------------------------------------------------------

void PageAdvanced::OnCheckSkipl0meshes() 
{
	applyOptions ();	
}

// ----------------------------------------------------------------------

void PageAdvanced::OnCheckDisabletexturebaking() 
{
	applyOptions ();
}

// ----------------------------------------------------------------------

void PageAdvanced::OnCheckDisablefilecaching() 
{
	applyOptions ();
}

// ----------------------------------------------------------------------

void PageAdvanced::OnCheckDisableasynchronousloader() 
{
	applyOptions ();
}

// ======================================================================

