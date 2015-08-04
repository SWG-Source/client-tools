// ======================================================================
//
// PageInformation.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "PageInformation.h"

#include "ClientMachine.h"
#include "Options.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================

bool copyTextToClipboard(const TCHAR *text)
{
	if (!OpenClipboard(AfxGetMainWnd ()->m_hWnd))
		return false;

	if (!EmptyClipboard())
		return false;

	// need to convert to cr/lf sequences.  yuck.
	int length = 1;
	for (const TCHAR *t2 = text; *t2; ++t2, ++length)
	{
		if (*t2 == '\n')
			++length;
	}

	HANDLE memoryHandle = GlobalAlloc(GMEM_MOVEABLE, length * sizeof(TCHAR));
	if (memoryHandle == NULL) 
	{ 
		CloseClipboard(); 
		return FALSE; 
	} 

	// lock the handle and copy the text to the buffer. 
	TCHAR *destination = reinterpret_cast<TCHAR *>(GlobalLock(memoryHandle));
	while (*text)
	{
		if (*text == '\n')
			*(destination++) = '\r';
		*destination++ = *text++;
	}

	if (GlobalUnlock(memoryHandle) != 0 || GetLastError() != NO_ERROR)
	{
		GlobalFree(memoryHandle);
		return false;
	}

	// TODO: May not work well with Win98SE
	if (!SetClipboardData(CF_UNICODETEXT, memoryHandle))
		return false;

	if (!CloseClipboard())
		return false;

	return true;
}

// ======================================================================

IMPLEMENT_DYNCREATE(PageInformation, CPropertyPage)

PageInformation::PageInformation() : CPropertyPage(PageInformation::IDD)
{
	//{{AFX_DATA_INIT(PageInformation)
	m_physicalMemorySize = _T("");
	m_numberOfProcessors = _T("");
	m_os = _T("");
	m_videoMemorySize = _T("");
	m_pixelShaderVersion = _T("");
	m_vertexShaderVersion = _T("");
	m_videoIdentifier = _T("");
	m_videoDriverVersion = _T("");
	m_videoDeviceId = _T("");
	m_cpuIdentifier = _T("");
	m_cpuSpeed = _T("");
	m_cpuVendor = _T("");
	m_directXVersion = _T("");
	m_trackIRVersion = _T("");

	m_lblNumProcessors = _T("");
	m_lblCpuIdentifier = _T("");
	m_lblCpuSpeed = _T("");
	m_lblCpuVendor = _T("");
	m_lblPhysicalMemorySize = _T("");
	m_lblVideoDescription = _T("");
	m_lblVideoIdentifier = _T("");
	m_lblVideoDriverVersion = _T("");
	m_lblVideoMemorySize = _T("");
	m_lblVertexShaderVersion = _T("");
	m_lblPixelShaderVersion = _T("");
	m_lblOperatingSystem = _T("");
	m_lblDirectXVersion = _T("");
	m_lblTrackIRVersion = _T("");

	m_lblBtnCopyToClipboard = _T("");
	//}}AFX_DATA_INIT
}

PageInformation::~PageInformation()
{
}

void PageInformation::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PageInformation)
	DDX_Text(pDX, IDC_STATIC_PHYSICAL, m_physicalMemorySize);
	DDX_Text(pDX, IDC_STATIC_NUMBEROFPROCESSORS, m_numberOfProcessors);
	DDX_Text(pDX, IDC_STATIC_OPERATINGSYSTEM, m_os);
	DDX_Text(pDX, IDC_STATIC_VIDEOMEMORYSIZE, m_videoMemorySize);
	DDX_Text(pDX, IDC_STATIC_PIXELSHADERVERSION, m_pixelShaderVersion);
	DDX_Text(pDX, IDC_STATIC_VERTEXSHADERVERSION, m_vertexShaderVersion);
	DDX_Text(pDX, IDC_STATIC_VIDEOIDENTIFIER, m_videoIdentifier);
	DDX_Text(pDX, IDC_STATIC_VIDEODRIVERVERSION, m_videoDriverVersion);
	DDX_Text(pDX, IDC_STATIC_VIDEODEVICEID, m_videoDeviceId);
	DDX_Text(pDX, IDC_STATIC_CPUIDENTIFIER, m_cpuIdentifier);
	DDX_Text(pDX, IDC_STATIC_CPUSPEED, m_cpuSpeed);
	DDX_Text(pDX, IDC_STATIC_CPUVENDOR, m_cpuVendor);
	DDX_Text(pDX, IDC_STATIC_DIRECTXVERSION, m_directXVersion);
	DDX_Text(pDX, IDC_STATIC_TRACKIRVERSION, m_trackIRVersion);

	DDX_Text(pDX, IDC_LBL_NUM_PROCESSORS, m_lblNumProcessors);
	DDX_Text(pDX, IDC_LBL_CPU_IDENTIFIER, m_lblCpuIdentifier);
	DDX_Text(pDX, IDC_LBL_CPU_SPEED, m_lblCpuSpeed);
	DDX_Text(pDX, IDC_LBL_CPU_VENDOR, m_lblCpuVendor);
	DDX_Text(pDX, IDC_LBL_PHYSICAL_MEMORY_SIZE, m_lblPhysicalMemorySize);
	DDX_Text(pDX, IDC_LBL_VIDEO_DESCRIPTION, m_lblVideoDescription);
	DDX_Text(pDX, IDC_LBL_VIDEO_IDENTIFIER, m_lblVideoIdentifier);
	DDX_Text(pDX, IDC_LBL_VIDEO_DRIVER_VERSION, m_lblVideoDriverVersion);
	DDX_Text(pDX, IDC_LBL_VIDEO_MEMORY_SIZE, m_lblVideoMemorySize);
	DDX_Text(pDX, IDC_LBL_VERTEX_SHADER_VERSION, m_lblVertexShaderVersion);
	DDX_Text(pDX, IDC_LBL_PIXEL_SHADER_VERSION, m_lblPixelShaderVersion);
	DDX_Text(pDX, IDC_LBL_OPERATING_SYSTEM, m_lblOperatingSystem);
	DDX_Text(pDX, IDC_LBL_DIRECTXVERSION, m_lblDirectXVersion);
	DDX_Text(pDX, IDC_LBL_TRACKIRVERSION, m_lblTrackIRVersion);

	DDX_Text(pDX, IDC_BUTTON_COPYTOCLIPBOARD, m_lblBtnCopyToClipboard);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(PageInformation, CPropertyPage)
	//{{AFX_MSG_MAP(PageInformation)
	ON_BN_CLICKED(IDC_BUTTON_COPYTOCLIPBOARD, OnButtonCopytoclipboard)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

void PageInformation::initializeDialog()
{
VERIFY(m_lblNumProcessors.LoadString(IDS_INFO_NUM_PROCESSORS));
	VERIFY(m_lblCpuIdentifier.LoadString(IDS_INFO_CPU_ID));
	VERIFY(m_lblCpuSpeed.LoadString(IDS_INFO_CPU_SPEED));
	VERIFY(m_lblCpuVendor.LoadString(IDS_INFO_CPU_VENDOR));
	VERIFY(m_lblPhysicalMemorySize.LoadString(IDS_INFO_PHYSICAL_MEMORY_SIZE));
	VERIFY(m_lblVideoDescription.LoadString(IDS_INFO_VIDEO_DESCRIPTION));
	VERIFY(m_lblVideoIdentifier.LoadString(IDS_INFO_VIDEO_IDENTIFIER));
	VERIFY(m_lblVideoDriverVersion.LoadString(IDS_INFO_VIDEO_DRIVER_VERSION));
	VERIFY(m_lblVideoMemorySize.LoadString(IDS_INFO_VIDEO_MEMORY_SIZE));
	VERIFY(m_lblVertexShaderVersion.LoadString(IDS_INFO_VERTEXT_SHADER_VERSION));
	VERIFY(m_lblPixelShaderVersion.LoadString(IDS_INFO_PIXEL_SHADER_VERSION));
	VERIFY(m_lblOperatingSystem.LoadString(IDS_INFO_OPERATING_SYSTEM));
	VERIFY(m_lblDirectXVersion.LoadString(IDS_INFO_DIRECTXVERSION));
	VERIFY(m_lblTrackIRVersion.LoadString(IDS_INFO_TRACKIRVERSION));
	VERIFY(m_lblBtnCopyToClipboard.LoadString(IDS_BTN_COPY_TO_CLIPBOARD));

	CString MB;
	VERIFY(MB.LoadString(IDS_MB));

	CString Mhz;
	VERIFY(Mhz.LoadString(IDS_Mhz));

	CString Unknown;
	VERIFY(Unknown.LoadString(IDS_UNKNOWN));

	m_physicalMemorySize.Format (_T("%i %s"), ClientMachine::getPhysicalMemorySize(), MB);

	m_numberOfProcessors.Format (_T("%i physical, %i logical"), ClientMachine::getNumberOfPhysicalProcessors (), ClientMachine::getNumberOfLogicalProcessors ());

	m_cpuVendor = ClientMachine::getCpuVendor ();

	m_cpuIdentifier = ClientMachine::getCpuIdentifier ();

	if(ClientMachine::getCpuSpeed() > 0)
		m_cpuSpeed.Format (_T("%i %s"), ClientMachine::getCpuSpeed (), Mhz);
	else
		m_cpuSpeed.Format (_T("%s %s"), Unknown, Mhz);

	m_os = ClientMachine::getOs ();

	m_videoIdentifier = ClientMachine::getDeviceDescription ();

	m_videoDeviceId.Format (_T("0x%04X[v], 0x%04X[d]"), ClientMachine::getVendorIdentifier (), ClientMachine::getDeviceIdentifier ());

	m_videoDriverVersion = ClientMachine::getDeviceDriverVersionText ();

	m_videoMemorySize.Format (_T("%i %s"), ClientMachine::getVideoMemorySize (), MB);

	if (ClientMachine::getVertexShaderMajorVersion () != 0)
		m_vertexShaderVersion.Format (_T("%i.%i"), ClientMachine::getVertexShaderMajorVersion (), ClientMachine::getVertexShaderMinorVersion ());
	else
		VERIFY(m_vertexShaderVersion.LoadString(IDS_NOT_SUPPORTED));

	if (ClientMachine::getPixelShaderMajorVersion () != 0)
		m_pixelShaderVersion.Format (_T("%i.%i"), ClientMachine::getPixelShaderMajorVersion (), ClientMachine::getPixelShaderMinorVersion ());
	else
		VERIFY(m_pixelShaderVersion.LoadString(IDS_NOT_SUPPORTED));

	m_directXVersion = ClientMachine::getDirectXVersion();

	m_trackIRVersion = ClientMachine::getTrackIRVersion();

	m_directXVersion = ClientMachine::getDirectXVersion();

	m_trackIRVersion = ClientMachine::getTrackIRVersion();

	UpdateData (false);
}

// ======================================================================

BOOL PageInformation::OnSetActive( )
{
	BOOL const result = CPropertyPage::OnSetActive();
	initializeDialog();
	return result;
}

// ======================================================================

BOOL PageInformation::OnInitDialog() 
{
	BOOL const result = CPropertyPage::OnInitDialog();
	initializeDialog();
	return result;
}

// ----------------------------------------------------------------------

void PageInformation::OnButtonCopytoclipboard() 
{
	CString const result = ClientMachine::getHardwareInformationString () + Options::getInformationString ();
	copyTextToClipboard (result);
}

// ======================================================================

