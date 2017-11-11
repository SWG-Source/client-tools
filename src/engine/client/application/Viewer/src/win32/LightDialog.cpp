// LightDialog.cpp : implementation file
//

#include "FirstViewer.h"
#include "LightDialog.h"

#include "ViewerDoc.h"

/////////////////////////////////////////////////////////////////////////////
// LightDialog dialog

#include "clientGraphics/Light.h"
#include "sharedMath/PackedRgb.h"

LightDialog::LightDialog(CWnd* pParent, Mode newMode, const LightManager& newLightManager, int newMyIndex)
	: CDialog(LightDialog::IDD, pParent),
	mode (newMode),
	lightManager (newLightManager),
	myIndex (newMyIndex)
{
	//{{AFX_DATA_INIT(LightDialog)
	m_colorB = 0.0f;
	m_colorG = 0.0f;
	m_colorR = 0.0f;
	m_disableLight = FALSE;
	m_dot3 = FALSE;
	m_shadowSource = FALSE;
	m_lightName = _T("");
	m_lightType = -1;
	m_pitch = 0;
	m_yaw = 0;
	m_posX = 0.0f;
	m_posY = 0.0f;
	m_posZ = 0.0f;
	m_colorBSpecular = 0.0f;
	m_colorGSpecular = 0.0f;
	m_colorRSpecular = 0.0f;
	m_attenuationConstant = 0.0f;
	m_attenuationLinear = 0.0f;
	m_attenuationQuadratic = 0.0f;
	m_range = 0.0f;
	m_spotFalloff = 0.0f;
	m_spotPhi = 0.0f;
	m_spotTheta = 0.0f;
	//}}AFX_DATA_INIT
}


void LightDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LightDialog)
	DDX_Control(pDX, IDC_STATIC_SPECULAR_COLOR, m_windowLightSpecular);
	DDX_Control(pDX, IDC_STATIC_LIGHT_COLOR, m_windowLightColor);
	DDX_Text(pDX, IDC_COLOR_B, m_colorB);
	DDX_Text(pDX, IDC_COLOR_G, m_colorG);
	DDX_Text(pDX, IDC_COLOR_R, m_colorR);
	DDX_Check(pDX, IDC_DISABLE_LIGHT, m_disableLight);
	DDX_Check(pDX, IDC_CHECK1, m_dot3);
	DDX_Check(pDX, IDC_CHECK2, m_shadowSource);
	DDX_Text(pDX, IDC_LIGHT_NAME, m_lightName);
	DDX_CBIndex(pDX, IDC_LIGHT_TYPE, m_lightType);
	DDX_Text(pDX, IDC_ORIENTATION_PITCH, m_pitch);
	DDV_MinMaxInt(pDX, m_pitch, -90, 90);
	DDX_Text(pDX, IDC_ORIENTATION_YAW, m_yaw);
	DDV_MinMaxInt(pDX, m_yaw, -180, 180);
	DDX_Text(pDX, IDC_POS_X, m_posX);
	DDX_Text(pDX, IDC_POS_Y, m_posY);
	DDX_Text(pDX, IDC_POS_Z, m_posZ);
	DDX_Text(pDX, IDC_COLOR_B_SPECULAR, m_colorBSpecular);
	DDX_Text(pDX, IDC_COLOR_G_SPECULAR, m_colorGSpecular);
	DDX_Text(pDX, IDC_COLOR_R_SPECULAR, m_colorRSpecular);
	DDX_Text(pDX, IDC_ATTENUATION_CONSTANT, m_attenuationConstant);
	DDX_Text(pDX, IDC_ATTENUATION_LINEAR, m_attenuationLinear);
	DDX_Text(pDX, IDC_ATTENUATION_QUADRATIC, m_attenuationQuadratic);
	DDX_Text(pDX, IDC_LIGHT_RANGE, m_range);
	DDX_Text(pDX, IDC_SPOT_FALLOFF, m_spotFalloff);
	DDX_Text(pDX, IDC_SPOT_PHI, m_spotPhi);
	DDX_Text(pDX, IDC_SPOT_THETA, m_spotTheta);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(LightDialog, CDialog)
	//{{AFX_MSG_MAP(LightDialog)
	ON_BN_CLICKED(IDC_BUTTON_LIGHT_COLOR, OnButtonLightColor)
	ON_BN_CLICKED(IDC_BUTTON_LIGHT_COLOR_SPECULAR, OnButtonLightColorSpecular)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// LightDialog message handlers

void LightDialog::OnOK() 
{
	UpdateData ();

	lightData.color.a = 1.0f;
	lightData.color.r = m_colorR;
	lightData.color.g = m_colorG;
	lightData.color.b = m_colorB;
	lightData.specularColor.a = 1.0f;
	lightData.specularColor.r = m_colorRSpecular;
	lightData.specularColor.g = m_colorGSpecular;
	lightData.specularColor.b = m_colorBSpecular;
	lightData.disabled      = m_disableLight != 0;
	lightData.dot3 = m_dot3 != 0;
	lightData.shadowSource = m_shadowSource != 0;
	strncpy (lightData.name, m_lightName, 100);
	lightData.phi           = convertDegreesToRadians (static_cast<real> (m_pitch));
	lightData.theta         = convertDegreesToRadians (static_cast<real> (m_yaw));
	lightData.position.set (CONST_REAL (m_posX), CONST_REAL (m_posY), CONST_REAL (m_posZ));
	lightData.type          = static_cast<Light::Type> (m_lightType);

	lightData.constantAttenuation  = m_attenuationConstant;
	lightData.linearAttenuation    = m_attenuationLinear;
	lightData.quadraticAttenuation = m_attenuationQuadratic;
	lightData.range                = m_range;               
	lightData.spotTheta            = m_spotTheta;           
	lightData.spotPhi              = m_spotPhi;             
	lightData.spotFalloff          = m_spotFalloff;

	//-- see if name is unique
	if (m_lightName.GetLength () > 0)
	{
		int i;
		for (i = 0; i < lightManager.getNumberOfLights (); i++)
			if (strcmp (lightManager.getLight (i).name, lightData.name) == 0)
				break;

		switch (mode)
		{
		case M_add:
			{
				if (i != lightManager.getNumberOfLights ())
					MessageBox ("Light name is already in use!", "Error");
				else
					CDialog::OnOK ();
			}
			break;

		case M_edit:
			{
				if (i != lightManager.getNumberOfLights () && i != myIndex)
					MessageBox ("Light name is already in use!", "Error");
				else
					CDialog::OnOK ();
			}
			break;
		}
	}
	else
		MessageBox ("Please specify a light name!", "Error");
}

BOOL LightDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	updateLightData ();
	UpdateData (false);

	Invalidate ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void LightDialog::updateLightData (void)
{
	m_colorR = lightData.color.r;
	m_colorG = lightData.color.g;
	m_colorB = lightData.color.b;

	m_colorRSpecular = lightData.specularColor.r;
	m_colorGSpecular = lightData.specularColor.g;
	m_colorBSpecular = lightData.specularColor.b;

	m_disableLight = lightData.disabled;
	m_dot3 = lightData.dot3;
	m_shadowSource = lightData.shadowSource;
	m_lightName = lightData.name;
	m_lightType = lightData.type;
	m_pitch = static_cast<int> (convertRadiansToDegrees (lightData.phi));
	m_yaw   = static_cast<int> (convertRadiansToDegrees (lightData.theta));
	m_posX  = lightData.position.x;
	m_posY  = lightData.position.y;
	m_posZ  = lightData.position.z;

	m_attenuationConstant  = lightData.constantAttenuation;
	m_attenuationLinear    = lightData.linearAttenuation;
	m_attenuationQuadratic = lightData.quadraticAttenuation;
	m_range                = lightData.range;
	m_spotTheta            = lightData.spotTheta;
	m_spotPhi              = lightData.spotPhi;
	m_spotFalloff          = lightData.spotFalloff;
}

void LightDialog::OnButtonLightColor() 
{
	UpdateData (true);

	// TODO: Add your control notification handler code here
	const int r = clamp(0, static_cast<int>(m_colorR * 255.0f), 255);
	const int g = clamp(0, static_cast<int>(m_colorG * 255.0f), 255);
	const int b = clamp(0, static_cast<int>(m_colorB * 255.0f), 255);
	CColorDialog dlg (RGB (r, g, b));

	if (dlg.DoModal () == IDOK)
	{
		COLORREF rgb = dlg.m_cc.rgbResult;
		m_colorR = static_cast<float>(GetRValue (rgb)) / 255.0f;
		m_colorG = static_cast<float>(GetGValue (rgb)) / 255.0f;
		m_colorB = static_cast<float>(GetBValue (rgb)) / 255.0f;

		UpdateData (false);
		Invalidate ();
	}
}

void LightDialog::OnButtonLightColorSpecular() 
{
	UpdateData (true);

	// TODO: Add your control notification handler code here
	const int r = clamp(0, static_cast<int>(m_colorRSpecular * 255.0f), 255);
	const int g = clamp(0, static_cast<int>(m_colorGSpecular * 255.0f), 255);
	const int b = clamp(0, static_cast<int>(m_colorBSpecular * 255.0f), 255);
	CColorDialog dlg (RGB (r, g, b));

	if (dlg.DoModal () == IDOK)
	{
		COLORREF rgb = dlg.m_cc.rgbResult;
		m_colorRSpecular = static_cast<float>(GetRValue (rgb)) / 255.0f;
		m_colorGSpecular = static_cast<float>(GetGValue (rgb)) / 255.0f;
		m_colorBSpecular = static_cast<float>(GetBValue (rgb)) / 255.0f;
		UpdateData (false);
		Invalidate ();
	}
}

void LightDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect rect;
	CBrush brush1;
	CBrush brush2;

	{
		const int r = clamp(0, static_cast<int>(m_colorR * 255.0f), 255);
		const int g = clamp(0, static_cast<int>(m_colorG * 255.0f), 255);
		const int b = clamp(0, static_cast<int>(m_colorB * 255.0f), 255);

		m_windowLightColor.GetWindowRect (&rect);
		ScreenToClient (&rect);
		brush1.CreateSolidBrush (RGB (r, g ,b));
		dc.SelectObject (&brush1);
		dc.Rectangle (&rect);
	}

	{
		const int r = clamp(0, static_cast<int>(m_colorRSpecular * 255.0f), 255);
		const int g = clamp(0, static_cast<int>(m_colorGSpecular * 255.0f), 255);
		const int b = clamp(0, static_cast<int>(m_colorBSpecular * 255.0f), 255);

		m_windowLightSpecular.GetWindowRect (&rect);
		ScreenToClient (&rect);
		brush2.CreateSolidBrush (RGB (r, g, b));
		dc.SelectObject (&brush2);
		dc.Rectangle (&rect);
	}
}

