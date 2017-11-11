//===================================================================
//
// DialogResetFloraParameters.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "FirstTerrainEditor.h"
#include "Terraineditor.h"

#include "DialogResetFloraParameters.h"

//===================================================================

DialogResetFloraParameters::DialogResetFloraParameters(bool radial, CWnd* pParent /*=NULL*/) :
	CDialog(DialogResetFloraParameters::IDD, pParent),
	m_radial (radial),
	m_shouldSway (FALSE),
	m_displacement (false),
	m_period (false),
	m_alignToTerrain (FALSE)
{
	//{{AFX_DATA_INIT(DialogResetFloraParameters)
	m_shouldSway = FALSE;
	m_alignToTerrain = FALSE;
	m_checkSway = FALSE;
	m_checkDisplacement = FALSE;
	m_checkPeriod = FALSE;
	m_checkAlign = FALSE;
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------

void DialogResetFloraParameters::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogResetFloraParameters)
	DDX_Control(pDX, IDC_EDIT_FREQUENCY, m_period);
	DDX_Control(pDX, IDC_EDIT_DISPLACEMENT, m_displacement);
	DDX_Check(pDX, IDC_CHECK_SWAY, m_shouldSway);
	DDX_Check(pDX, IDC_ALIGN_TO_TERRAIN, m_alignToTerrain);
	DDX_Check(pDX, IDC_CHECK_ALIGNTOTERRAIN, m_checkSway);
	DDX_Check(pDX, IDC_CHECK_CHANGE_DISPLACEMENT, m_checkDisplacement);
	DDX_Check(pDX, IDC_CHECK_CHANGE_PERIOD, m_checkPeriod);
	DDX_Check(pDX, IDC_CHECK_CHANGE_SWAY, m_checkAlign);
	//}}AFX_DATA_MAP
}

//===================================================================

BEGIN_MESSAGE_MAP(DialogResetFloraParameters, CDialog)
	//{{AFX_MSG_MAP(DialogResetFloraParameters)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//===================================================================

BOOL DialogResetFloraParameters::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_checkDisplacement = TRUE;
	m_displacement      = m_radial ? 0.15f : 0.01f;

	m_checkPeriod       = TRUE;
	m_period            = m_radial ? 0.5f : 0.1f;

	m_checkSway         = TRUE;
	m_shouldSway        = TRUE;

	m_checkAlign        = TRUE;
	m_alignToTerrain    = TRUE;

	UpdateData (false);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//-------------------------------------------------------------------

void DialogResetFloraParameters::OnOK() 
{
	UpdateData (true);

	m_finalDisplacement = m_displacement;
	m_finalPeriod       = m_period;
	
	CDialog::OnOK();
}

//===================================================================

