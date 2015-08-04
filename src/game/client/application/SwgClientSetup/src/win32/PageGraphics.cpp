// ======================================================================
//
// PageGraphics.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "FirstSwgClientSetup.h"
#include "PageGraphics.h"

#include "ClientMachine.h"
#include "Options.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ======================================================================

IMPLEMENT_DYNCREATE(PageGraphics, CPropertyPage)

PageGraphics::PageGraphics() : CPropertyPage(PageGraphics::IDD)
{
	//{{AFX_DATA_INIT(PageGraphics)
	m_lblGameResolution = _T("");
	m_lblPixelShaderVersion = _T("");

	m_lblWindowedMode = _T("");
	m_lblBorderlessWindow = _T("");
	m_lblDisableBumpMapping = _T("");
	m_lblDisableHardwareMouseControl = _T("");
	m_lblUseLowDetailTextures = _T("");
	m_lblUseLowDetailNormalMaps = _T("");
	m_lblDisableMultipassRendering = _T("");
	m_lblDisableVsync = _T("");
	m_lblDisableFastMouseCursor = _T("");
	m_lblUseSafeRenderer = _T("");

	m_windowed = FALSE;
	m_borderlessWindow = FALSE;
	m_disableBumpMapping = FALSE;
	m_disableHardwareMouseCursor = FALSE;
	m_useLowDetailTextures = FALSE;
	m_useLowDetailNormalMaps = FALSE;
	m_disableMultipassRendering = FALSE;
	m_disableVsync = FALSE;
	m_disableFastMouseCursor = FALSE;
	m_useSafeRenderer = FALSE;
	//}}AFX_DATA_INIT
}

PageGraphics::~PageGraphics()
{
}

void PageGraphics::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PageGraphics)

	DDX_Text(pDX, IDC_LBL_GRAPH_GAME_RESOLUTION, m_lblGameResolution);
	DDX_Text(pDX, IDC_GRAPH_LBL_PIXEL_SHADER_VERSION, m_lblPixelShaderVersion);

	DDX_Text(pDX, IDC_CHECK_WINDOWED_MODE, m_lblWindowedMode);
	DDX_Text(pDX, IDC_CHECK_BORDERLESSWINDOW, m_lblBorderlessWindow);
	DDX_Text(pDX, IDC_CHECK_DISABLEBUMP, m_lblDisableBumpMapping);
	DDX_Text(pDX, IDC_CHECK_DISABLEHARDWAREMOUSECURSOR, m_lblDisableHardwareMouseControl);
	DDX_Text(pDX, IDC_CHECK_USE_LOW_DETAIL_TEXTURES, m_lblUseLowDetailTextures);
	DDX_Text(pDX, IDC_CHECK_USE_LOW_DETAIL_NORMAL_MAPS, m_lblUseLowDetailNormalMaps);
	DDX_Text(pDX, IDC_CHECK_DISABLE_MULTIPASS_RENDERING, m_lblDisableMultipassRendering);
	DDX_Text(pDX, IDC_CHECK_DISABLE_VSYNC, m_lblDisableVsync);
	DDX_Text(pDX, IDC_CHECK_DISABLE_FAST_MOUSE_CURSOR, m_lblDisableFastMouseCursor);
	DDX_Text(pDX, IDC_CHECK_USE_SAFE_RENDERER, m_lblUseSafeRenderer);

	DDX_Control(pDX, IDC_RESOLUTION, m_resolution);
	DDX_Control(pDX, IDC_VERTEXPIXELSHADERVERSION, m_vertexPixelShaderVersion);
	
	DDX_Check(pDX, IDC_CHECK_WINDOWED_MODE, m_windowed);
	DDX_Check(pDX, IDC_CHECK_BORDERLESSWINDOW, m_borderlessWindow);
    DDX_Control(pDX, IDC_CHECK_DISABLEBUMP, m_disableBumpMappingButton);
	DDX_Check(pDX,   IDC_CHECK_DISABLEBUMP, m_disableBumpMapping);
    DDX_Control(pDX, IDC_CHECK_DISABLEHARDWAREMOUSECURSOR, m_disableHardwareMouseCursorButton);
	DDX_Check(pDX,   IDC_CHECK_DISABLEHARDWAREMOUSECURSOR, m_disableHardwareMouseCursor);
	DDX_Check(pDX, IDC_CHECK_USE_LOW_DETAIL_TEXTURES, m_useLowDetailTextures);
	DDX_Check(pDX, IDC_CHECK_USE_LOW_DETAIL_NORMAL_MAPS, m_useLowDetailNormalMaps);
	DDX_Check(pDX, IDC_CHECK_DISABLE_MULTIPASS_RENDERING, m_disableMultipassRendering);
	DDX_Check(pDX, IDC_CHECK_DISABLE_VSYNC, m_disableVsync);
	DDX_Check(pDX, IDC_CHECK_DISABLE_FAST_MOUSE_CURSOR, m_disableFastMouseCursor);
	DDX_Check(pDX, IDC_CHECK_USE_SAFE_RENDERER, m_useSafeRenderer);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(PageGraphics, CPropertyPage)
	//{{AFX_MSG_MAP(PageGraphics)
	ON_CBN_SELCHANGE(IDC_RESOLUTION, OnSelchangeResolution)
	ON_CBN_SELCHANGE(IDC_VERTEXPIXELSHADERVERSION, OnSelchangeVertexpixelshaderversion)

	ON_BN_CLICKED(IDC_CHECK_WINDOWED_MODE, OnCheckWindowed)
	ON_BN_CLICKED(IDC_CHECK_BORDERLESSWINDOW, OnCheckBorderlesswindow)
	ON_BN_CLICKED(IDC_CHECK_DISABLEBUMP, OnCheckDisablebump)
	ON_BN_CLICKED(IDC_CHECK_DISABLEHARDWAREMOUSECURSOR, OnCheckDisablehardwaremousecursor)
	ON_BN_CLICKED(IDC_CHECK_USE_LOW_DETAIL_TEXTURES, OnCheckUselowdetailtextures)
	ON_BN_CLICKED(IDC_CHECK_USE_LOW_DETAIL_NORMAL_MAPS, OnCheckUselowdetailnormalmaps)
	ON_BN_CLICKED(IDC_CHECK_DISABLE_MULTIPASS_RENDERING, OnCheckDisablemultipassrendering)
	ON_BN_CLICKED(IDC_CHECK_DISABLE_VSYNC, OnCheckDisablevsync)
	ON_BN_CLICKED(IDC_CHECK_DISABLE_FAST_MOUSE_CURSOR, OnCheckDisablefastmousecursor)
	ON_BN_CLICKED(IDC_CHECK_USE_SAFE_RENDERER, OnCheckUsesaferenderer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

void PageGraphics::initializeDialog() 
{

	VERIFY(m_lblGameResolution.LoadString(IDS_GRAPHICS_GAME_RESOLUTION));
	VERIFY(m_lblPixelShaderVersion.LoadString(IDS_GRAPHICS_PIXEL_SHADER_VERSION));
	VERIFY(m_lblWindowedMode.LoadString(IDS_GRAPHICS_WINDOWED_MODE));
	VERIFY(m_lblBorderlessWindow.LoadString(IDS_GRAPHICS_BORDERLESS_WINDOW));
	VERIFY(m_lblDisableBumpMapping.LoadString(IDS_GRAPHICS_DISABLE_BUMP_MAPPING));
	VERIFY(m_lblDisableHardwareMouseControl.LoadString(IDS_GRAPHICS_DISABLE_HARDWARE_MOUSE_CURSOR));
	VERIFY(m_lblUseLowDetailTextures.LoadString(IDS_GRAPHICS_USE_LOW_DETAIL_TEXTURES));
	VERIFY(m_lblUseLowDetailNormalMaps.LoadString(IDS_GRAPHICS_USE_LOW_DETAIL_NORMAL_MAPS));
	VERIFY(m_lblDisableMultipassRendering.LoadString(IDS_GRAPHICS_DISABLE_MULTIPASS_RENDERING));
	VERIFY(m_lblDisableVsync.LoadString(IDS_GRAPHICS_DISABLE_VSYNC));
	VERIFY(m_lblDisableFastMouseCursor.LoadString(IDS_GRAPHICS_DISABLE_FAST_MOUSE_CURSOR));
	VERIFY(m_lblUseSafeRenderer.LoadString(IDS_GRAPHICS_USE_SAFE_RENDERER));
	VERIFY(m_lblVspsOptimal.LoadString(IDS_VSPS_OPTIMAL));
	VERIFY(m_lblVsps20.LoadString(IDS_VSPS_2_0));
	VERIFY(m_lblVsps14.LoadString(IDS_VSPS_1_4));
	VERIFY(m_lblVsps11.LoadString(IDS_VSPS_1_1));
	VERIFY(m_lblVspsDisabled.LoadString(IDS_VSPS_DISABLED));

	
	UpdateData (false);
	
}
// ======================================================================

BOOL PageGraphics::OnSetActive( )
{
	BOOL const result = CPropertyPage::OnSetActive();
	initializeDialog();
	return result;
}

// ======================================================================

BOOL PageGraphics::OnInitDialog() 
{
	BOOL const result = CPropertyPage::OnInitDialog();

	initializeDialog();

	//-- resolutions
	int const screenWidth = Options::getScreenWidth ();
	int const screenHeight = Options::getScreenHeight ();
	int const refreshRate = Options::getFullScreenRefreshRate ();

	//-- select the appropriate refresh rate
	{
		int selection = 0;

		for (int i = 0; i < ClientMachine::getNumberOfDisplayModes (); ++i)
		{
			int const displayModeWidth = ClientMachine::getDisplayModeWidth (i);
			int const displayModeHeight = ClientMachine::getDisplayModeHeight (i);
			int const displayModeRefreshRate = ClientMachine::getDisplayModeRefreshRate (i);

			TCHAR buffer [256];
			_stprintf (buffer, _T("%i x %i @ %i Hz"), displayModeWidth, displayModeHeight, displayModeRefreshRate);

			m_resolution.AddString (buffer);

			if (screenWidth == displayModeWidth && screenHeight == displayModeHeight && refreshRate == displayModeRefreshRate)
				selection = i;
		}

		if (selection < ClientMachine::getNumberOfDisplayModes ())
			m_resolution.SetCurSel (selection);
	}

	//-- vertex and pixel shader version
	{
		int selection = 0;
		if ((ClientMachine::getPixelShaderMajorVersion () > 0) && (ClientMachine::getPhysicalMemorySize() > 260))
		{
			int item = m_vertexPixelShaderVersion.AddString (m_lblVspsOptimal);
			m_vertexPixelShaderVersion.SetItemData (item, static_cast<DWORD> (-1));
		
			int const supportedPixelShaderMajorVersion = ClientMachine::getPixelShaderMajorVersion ();
			int const supportedPixelShaderMinorVersion = ClientMachine::getPixelShaderMinorVersion ();

			int const pixelShaderMajorVersion = Options::getPixelShaderMajorVersion ();
			int const pixelShaderMinorVersion = Options::getPixelShaderMinorVersion ();

			if (supportedPixelShaderMajorVersion >= 2)
			{
				item = m_vertexPixelShaderVersion.AddString (m_lblVsps20);
				m_vertexPixelShaderVersion.SetItemData (item, 20);

				item = m_vertexPixelShaderVersion.AddString (m_lblVsps14);
				m_vertexPixelShaderVersion.SetItemData (item, 14);

				item = m_vertexPixelShaderVersion.AddString (m_lblVsps11);
				m_vertexPixelShaderVersion.SetItemData (item, 11);

				switch (pixelShaderMajorVersion)
				{
				case -1:  
					selection = 0;  
					break;

				case 2:   
					selection = 1;  
					break;

				case 1:   
					{
						if (pixelShaderMinorVersion == 4)
							selection = 2;
						else
							selection = 3;
					}
					break;

				case 0:   
					selection = 4;
					break;
				}
			}
			else
				if (supportedPixelShaderMajorVersion == 1)
				{
					if (supportedPixelShaderMinorVersion == 4)
					{
						item = m_vertexPixelShaderVersion.AddString (m_lblVsps14);
						m_vertexPixelShaderVersion.SetItemData (item, 14);

						item = m_vertexPixelShaderVersion.AddString (m_lblVsps11);
						m_vertexPixelShaderVersion.SetItemData (item, 11);

						switch (pixelShaderMajorVersion)
						{
						case -1:  
							selection = 0;  
							break;

						case 1:   
							{
								if (pixelShaderMinorVersion == 4)
									selection = 1;
								else
									selection = 2;
							}
							break;

						case 0:   
							selection = 3;
							break;
						}
					}
					else
					{
						item = m_vertexPixelShaderVersion.AddString (m_lblVsps11);
						m_vertexPixelShaderVersion.SetItemData (item, 11);

						switch (pixelShaderMajorVersion)
						{
						case 1:   
							selection = 1;
							break;

						case 0:   
							selection = 2;
							break;
						}
					}
				}
				else
				{
					Options::setPixelShaderVersion(0, 0);  
					m_vertexPixelShaderVersion.EnableWindow (false);
					selection = 0;
				}
		}
		else
		{
			Options::setPixelShaderVersion(0, 0);  
			m_vertexPixelShaderVersion.EnableWindow (false);
			selection = 0;
		}

		int const item = m_vertexPixelShaderVersion.AddString (m_lblVspsDisabled);
		m_vertexPixelShaderVersion.SetItemData (item, 0);

		m_vertexPixelShaderVersion.SetCurSel (selection);
	}

	//-- hardware mouse cursor
	m_disableHardwareMouseCursor = ClientMachine::getSupportsHardwareMouseCursor () ? Options::getDisableHardwareMouseCursor () : TRUE;
	m_disableHardwareMouseCursorButton.EnableWindow (ClientMachine::getSupportsHardwareMouseCursor ());

	//-- other options
	m_disableBumpMapping = Options::getDisableBumpMapping ();

	m_windowed = Options::getWindowed ();
	m_borderlessWindow = Options::getBorderlessWindow ();

	m_useLowDetailTextures = Options::getDiscardHighestMipMapLevels ();
	m_useLowDetailNormalMaps = Options::getDiscardHighestNormalMipMapLevels ();

	m_disableMultipassRendering = Options::getDisableMultipassRendering ();
	m_disableVsync = Options::getAllowTearing ();

	m_disableFastMouseCursor = Options::getDisableFastMouseCursor ();
	m_useSafeRenderer = Options::getUseSafeRenderer ();

	if (ClientMachine::getPhysicalMemorySize() < 260)
	{
		//Set disable bump mapping, low detail textures, low detail normal maps, and disable
		//those controls
		Options::setDisableBumpMapping(true);
		m_disableBumpMapping = true;
		m_disableBumpMappingButton.EnableWindow(false);

		Options::setDiscardHighestMipMapLevels(true);

		Options::setDiscardHighestNormalMipMapLevels(true);
	}
	else if (ClientMachine::getPhysicalMemorySize() < 400)
	{
		//Set disable bump mapping, low detail textures, low detail normal maps
		Options::setDisableBumpMapping(true);
		m_disableBumpMapping = true;
		
		Options::setDiscardHighestMipMapLevels(true);
	
		Options::setDiscardHighestNormalMipMapLevels(true);
	}

	return result;
}

// ----------------------------------------------------------------------

void PageGraphics::applyOptions ()
{
	UpdateData (true);

	int const selection = m_resolution.GetCurSel ();
	if (selection != CB_ERR)
	{
		Options::setScreenWidth (ClientMachine::getDisplayModeWidth (selection));
		Options::setScreenHeight (ClientMachine::getDisplayModeHeight (selection));
		Options::setFullScreenRefreshRate (ClientMachine::getDisplayModeRefreshRate (selection));
	}

	switch (m_vertexPixelShaderVersion.GetItemData (m_vertexPixelShaderVersion.GetCurSel ()))
	{
	case -1:  
		Options::setPixelShaderVersion (-1, -1);  
		break;

	case 11:
		Options::setPixelShaderVersion (1, 1);  
		break;

	case 14:
		Options::setPixelShaderVersion (1, 4);  
		break;

	case 20:
		Options::setPixelShaderVersion (2, 0);  
		break;

	case 0:
	default:
		Options::setPixelShaderVersion (0, 0);  
		break;
	}

	Options::setWindowed (m_windowed == TRUE);
	Options::setBorderlessWindow (m_borderlessWindow == TRUE);
	Options::setDisableBumpMapping (m_disableBumpMapping == TRUE);
	Options::setDisableHardwareMouseCursor (m_disableHardwareMouseCursor == TRUE);
	Options::setDiscardHighestMipMapLevels (m_useLowDetailTextures == TRUE);
	Options::setDiscardHighestNormalMipMapLevels (m_useLowDetailNormalMaps == TRUE);
	Options::setDisableMultipassRendering (m_disableMultipassRendering == TRUE);
	Options::setAllowTearing (m_disableVsync == TRUE);
	Options::setDisableFastMouseCursor (m_disableFastMouseCursor == TRUE);
	Options::setUseSafeRenderer (m_useSafeRenderer == TRUE);
}

// ----------------------------------------------------------------------

void PageGraphics::OnSelchangeResolution() 
{
	applyOptions ();
}

// ----------------------------------------------------------------------

void PageGraphics::OnSelchangeVertexpixelshaderversion() 
{
	applyOptions ();
}

// ----------------------------------------------------------------------

void PageGraphics::OnCheckWindowed() 
{
	applyOptions ();
}

// ----------------------------------------------------------------------

void PageGraphics::OnCheckBorderlesswindow() 
{
	applyOptions ();
}

// ----------------------------------------------------------------------

void PageGraphics::OnCheckDisablebump() 
{
	applyOptions ();	
}

// ----------------------------------------------------------------------

void PageGraphics::OnCheckDisablehardwaremousecursor() 
{
	applyOptions ();
}

// ----------------------------------------------------------------------

void PageGraphics::OnCheckUselowdetailtextures() 
{
	applyOptions ();
}

// ----------------------------------------------------------------------

void PageGraphics::OnCheckUselowdetailnormalmaps() 
{
	applyOptions ();
}

// ----------------------------------------------------------------------

void PageGraphics::OnCheckDisablemultipassrendering() 
{
	applyOptions ();
}

// ----------------------------------------------------------------------

void PageGraphics::OnCheckDisablevsync() 
{
	applyOptions ();
}

// ----------------------------------------------------------------------

void PageGraphics::OnCheckDisablefastmousecursor() 
{
	applyOptions ();
}

// ----------------------------------------------------------------------

void PageGraphics::OnCheckUsesaferenderer() 
{
	applyOptions ();
}

// ----------------------------------------------------------------------

BOOL PageGraphics::OnKillActive() 
{
	// TODO: Add your specialized code here and/or call the base class
	applyOptions ();
	
	return CPropertyPage::OnKillActive();
}

// ======================================================================

