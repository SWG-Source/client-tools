//===================================================================
//
// PreferencesDialog.cpp
// copyright 1999, Bootprint Entertainment
// copyright 2002, sony online entertainment
//
//===================================================================

#include "FirstViewer.h"
#include "PreferencesDialog.h"

#include "ViewerPreferences.h"
#include "sharedMath/PackedRgb.h"
#include "clientGame/Bloom.h"

//===================================================================

PreferencesDialog::PreferencesDialog(CWnd* pParent)
	: CDialog(PreferencesDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(PreferencesDialog)
	m_alternateRGB = FALSE;
	m_colorB = 0;
	m_colorG = 0;
	m_colorR = 0;
	m_gridSize = 0;
	m_numSubdivisions = 0;
	m_cameraMode = -1;
	m_lowPriority = FALSE;
	m_enableFog = FALSE;
	m_displayRenderStats = FALSE;
	m_fogB = 0;
	m_fogDensity = 0.0f;
	m_fogG = 0;
	m_fogR = 0;
	m_environmentMapName = _T("");
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------

void PreferencesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PreferencesDialog)
	DDX_Control(pDX, IDC_BLOOM, m_bloom);
	DDX_Control(pDX, IDC_STATIC_FOG_COLOR, m_fogColor);
	DDX_Control(pDX, IDC_STATIC_COLOR_WINDOW, m_windowColor);
	DDX_Check(pDX, IDC_CHECK_ALTERNATERGB, m_alternateRGB);
	DDX_Text(pDX, IDC_COLOR_B, m_colorB);
	DDV_MinMaxInt(pDX, m_colorB, 0, 255);
	DDX_Text(pDX, IDC_COLOR_G, m_colorG);
	DDV_MinMaxInt(pDX, m_colorG, 0, 255);
	DDX_Text(pDX, IDC_COLOR_R, m_colorR);
	DDV_MinMaxInt(pDX, m_colorR, 0, 255);
	DDX_Text(pDX, IDC_EDIT_GRID_SIZE, m_gridSize);
	DDX_Text(pDX, IDC_EDIT_NUM_SUBDIVISIONS, m_numSubdivisions);
	DDX_Radio(pDX, IDC_RADIO_MAYA, m_cameraMode);
	DDX_Check(pDX, IDC_LOWPRIORITY, m_lowPriority);
	DDX_Check(pDX, IDC_CHECK_ENABLEFOG, m_enableFog);
	DDX_Check(pDX, IDC_DISPLAY_RENDER_STATS, m_displayRenderStats);
	DDX_Text(pDX, IDC_FOG_B, m_fogB);
	DDX_Text(pDX, IDC_FOG_DENSITY, m_fogDensity);
	DDX_Text(pDX, IDC_FOG_G, m_fogG);
	DDX_Text(pDX, IDC_FOG_R, m_fogR);
	DDX_Text(pDX, IDC_EDIT_ENVIRONMENTMAPNAME, m_environmentMapName);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(PreferencesDialog, CDialog)
	//{{AFX_MSG_MAP(PreferencesDialog)
	ON_BN_CLICKED(IDC_BUTTON_LIGHT_COLOR, OnButtonLightColor)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_FOG_COLOR, OnButtonFogColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

BOOL PreferencesDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	readPreferences ();
	if (!Bloom::isSupported())
	{
		m_bloom.SetCheck(0);
		m_bloom.EnableWindow(false);
	}

	UpdateData (false);

	return TRUE;
}

//-------------------------------------------------------------------

void PreferencesDialog::OnOK() 
{
	UpdateData (true);
	writePreferences ();

	CDialog::OnOK();
}

//===================================================================

void PreferencesDialog::OnButtonFogColor() 
{
	UpdateData (true);

	// TODO: Add your control notification handler code here
	CColorDialog dlg (RGB (m_fogR, m_fogG, m_fogB));

	if (dlg.DoModal () == IDOK)
	{
		COLORREF rgb = dlg.m_cc.rgbResult;
		m_fogR = GetRValue (rgb);
		m_fogG = GetGValue (rgb);
		m_fogB = GetBValue (rgb);

		UpdateData (false);
		Invalidate ();
	}
}

//-------------------------------------------------------------------

void PreferencesDialog::OnButtonLightColor() 
{
	UpdateData (true);

	CColorDialog dlg (RGB (m_colorR, m_colorG, m_colorB));

	if (dlg.DoModal () == IDOK)
	{
		COLORREF rgb = dlg.m_cc.rgbResult;
		m_colorR = GetRValue (rgb);
		m_colorG = GetGValue (rgb);
		m_colorB = GetBValue (rgb);

		UpdateData (false);
		Invalidate ();
	}
}

//-------------------------------------------------------------------

void PreferencesDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	{
		CRect rect;
		CBrush brush;
		m_windowColor.GetWindowRect (&rect);
		ScreenToClient (&rect);
		brush.CreateSolidBrush (RGB (m_colorR, m_colorG, m_colorB));
		dc.SelectObject (&brush);
		dc.Rectangle (&rect);
	}

	{
		CRect rect;
		CBrush brush;
		m_fogColor.GetWindowRect (&rect);
		ScreenToClient (&rect);
		brush.CreateSolidBrush (RGB (m_fogR, m_fogG, m_fogB));
		dc.SelectObject (&brush);
		dc.Rectangle (&rect);
	}
}

//===================================================================

void PreferencesDialog::readPreferences ()
{
	m_alternateRGB = ViewerPreferences::getAlternateRgb ();

	PackedRgb color;
	color.convert (ViewerPreferences::getClearColor ());

	m_colorR = color.r;
	m_colorG = color.g;
	m_colorB = color.b;

	m_cameraMode = ViewerPreferences::getCameraMode ();

	m_lowPriority = ViewerPreferences::getLowPriority ();

	color.convert (ViewerPreferences::getFogColor ());

	m_fogR = color.r;
	m_fogG = color.g;
	m_fogB = color.b;

	m_enableFog  = ViewerPreferences::getFogEnabled ();
	m_fogDensity = ViewerPreferences::getFogDensity ();

	m_environmentMapName = ViewerPreferences::getEnvironmentMapName ();

	if (Bloom::isSupported() && ViewerPreferences::getBloomEnabled())
		m_bloom.SetCheck(1);
	else
		m_bloom.SetCheck(0);
}

//-------------------------------------------------------------------

void PreferencesDialog::writePreferences () const
{
	PackedRgb clearColor;
	clearColor.r = static_cast<uint8> (m_colorR);
	clearColor.g = static_cast<uint8> (m_colorG);
	clearColor.b = static_cast<uint8> (m_colorB);

	PackedRgb fogColor;
	fogColor.r = static_cast<uint8> (m_fogR);
	fogColor.g = static_cast<uint8> (m_fogG);
	fogColor.b = static_cast<uint8> (m_fogB);

	ViewerPreferences::setAlternateRgb (m_alternateRGB == TRUE);
	ViewerPreferences::setClearColor (clearColor.convert (0.0f));
	ViewerPreferences::setCameraMode (static_cast<ViewerPreferences::CameraMode> (m_cameraMode));
	ViewerPreferences::setLowPriority (m_lowPriority == TRUE);
	ViewerPreferences::setFogEnabled (m_enableFog == TRUE);
	ViewerPreferences::setFogColor (fogColor.convert ());
	ViewerPreferences::setFogDensity (m_fogDensity);

	ViewerPreferences::setEnvironmentMapName (m_environmentMapName);
	ViewerPreferences::setBloomEnabled(m_bloom.GetCheck() == 1);
}

//===================================================================

