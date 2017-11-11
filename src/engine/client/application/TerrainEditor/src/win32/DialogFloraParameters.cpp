//===================================================================
//
// DialogFloraParameters.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "FirstTerrainEditor.h"
#include "DialogFloraParameters.h"

//===================================================================

DialogFloraParameters::DialogFloraParameters(CWnd* pParent /*=NULL*/)
	: CDialog(DialogFloraParameters::IDD, pParent)
{
	//{{AFX_DATA_INIT(DialogFloraParameters)
	m_collidableMinimumDistance = 0.0f;
	m_collidableMaximumDistance = 0.0f;
	m_collidableSeed = 0;
	m_collidableTileBorder = 0.0f;
	m_collidableTileSize = 0.0f;
	m_collidableTotalFlora = 0;
	m_nonCollidableMinimumDistance = 0.0f;
	m_nonCollidableMaximumDistance = 0.0f;
	m_nonCollidableSeed = 0;
	m_nonCollidableTileBorder = 0.0f;
	m_nonCollidableTileSize = 0.0f;
	m_nonCollidableTotalFlora = 0;
	m_radialMinimumDistance = 0.0f;
	m_radialMaximumDistance = 0.0f;
	m_radialSeed = 0;
	m_radialTileBorder = 0.0f;
	m_radialTileSize = 0.0f;
	m_radialTotalFlora = 0;
	m_farRadialMinimumDistance = 0.0f;
	m_farRadialMaximumDistance = 0.0f;
	m_farRadialSeed = 0;
	m_farRadialTileBorder = 0.0f;
	m_farRadialTileSize = 0.0f;
	m_farRadialTotalFlora = 0;
	//}}AFX_DATA_INIT
}

//===================================================================

void DialogFloraParameters::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogFloraParameters)
	DDX_Text(pDX, IDC_EDIT_C_MINDISTANCE, m_collidableMinimumDistance);
	DDX_Text(pDX, IDC_EDIT_C_MAXDISTANCE, m_collidableMaximumDistance);
	DDX_Text(pDX, IDC_EDIT_C_SEED, m_collidableSeed);
	DDX_Text(pDX, IDC_EDIT_C_TILEBORDER, m_collidableTileBorder);
	DDX_Text(pDX, IDC_EDIT_C_TILESIZE, m_collidableTileSize);
	DDX_Text(pDX, IDC_EDIT_C_TOTALFLORA, m_collidableTotalFlora);
	DDX_Text(pDX, IDC_EDIT_N_MINDISTANCE, m_nonCollidableMinimumDistance);
	DDX_Text(pDX, IDC_EDIT_N_MAXDISTANCE, m_nonCollidableMaximumDistance);
	DDX_Text(pDX, IDC_EDIT_N_SEED, m_nonCollidableSeed);
	DDX_Text(pDX, IDC_EDIT_N_TILEBORDER, m_nonCollidableTileBorder);
	DDX_Text(pDX, IDC_EDIT_N_TILESIZE, m_nonCollidableTileSize);
	DDX_Text(pDX, IDC_EDIT_N_TOTALFLORA, m_nonCollidableTotalFlora);
	DDX_Text(pDX, IDC_EDIT_R_MINDISTANCE, m_radialMinimumDistance);
	DDX_Text(pDX, IDC_EDIT_R_MAXDISTANCE, m_radialMaximumDistance);
	DDX_Text(pDX, IDC_EDIT_R_SEED, m_radialSeed);
	DDX_Text(pDX, IDC_EDIT_R_TILEBORDER, m_radialTileBorder);
	DDX_Text(pDX, IDC_EDIT_R_TILESIZE, m_radialTileSize);
	DDX_Text(pDX, IDC_EDIT_R_TOTALFLORA, m_radialTotalFlora);
	DDX_Text(pDX, IDC_EDIT_F_MINDISTANCE, m_farRadialMinimumDistance);
	DDX_Text(pDX, IDC_EDIT_F_MAXDISTANCE, m_farRadialMaximumDistance);
	DDX_Text(pDX, IDC_EDIT_F_SEED, m_farRadialSeed);
	DDX_Text(pDX, IDC_EDIT_F_TILEBORDER, m_farRadialTileBorder);
	DDX_Text(pDX, IDC_EDIT_F_TILESIZE, m_farRadialTileSize);
	DDX_Text(pDX, IDC_EDIT_F_TOTALFLORA, m_farRadialTotalFlora);
	//}}AFX_DATA_MAP
}

//===================================================================

BEGIN_MESSAGE_MAP(DialogFloraParameters, CDialog)
	//{{AFX_MSG_MAP(DialogFloraParameters)
	ON_EN_CHANGE(IDC_EDIT_C_MAXDISTANCE, recalculate)
	ON_EN_CHANGE(IDC_EDIT_C_MINDISTANCE, recalculate)
	ON_EN_CHANGE(IDC_EDIT_C_TILESIZE, recalculate)
	ON_EN_CHANGE(IDC_EDIT_N_MAXDISTANCE, recalculate)
	ON_EN_CHANGE(IDC_EDIT_N_MINDISTANCE, recalculate)
	ON_EN_CHANGE(IDC_EDIT_N_TILESIZE, recalculate)
	ON_EN_CHANGE(IDC_EDIT_R_MAXDISTANCE, recalculate)
	ON_EN_CHANGE(IDC_EDIT_R_MINDISTANCE, recalculate)
	ON_EN_CHANGE(IDC_EDIT_R_TILESIZE, recalculate)
	ON_EN_CHANGE(IDC_EDIT_F_MAXDISTANCE, recalculate)
	ON_EN_CHANGE(IDC_EDIT_F_MINDISTANCE, recalculate)
	ON_EN_CHANGE(IDC_EDIT_F_TILESIZE, recalculate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//===================================================================

BOOL DialogFloraParameters::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	recalculate ();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//===================================================================

int computeTotalFlora (float maximumDistance, float minimumDistance, float tileSize)
{
	if (tileSize <= 0.f)
		return 0;

	if (maximumDistance <= 0)
		return 0;

	if (maximumDistance < minimumDistance)
		return 0;

	return static_cast<int> (PI * (sqr (2.f * maximumDistance / tileSize) - sqr (2.f * minimumDistance / tileSize)) / 4);
}

//-------------------------------------------------------------------

void DialogFloraParameters::recalculate ()
{
	UpdateData (true);

	m_collidableTotalFlora    = computeTotalFlora (m_collidableMaximumDistance, m_collidableMinimumDistance, m_collidableTileSize);
	m_nonCollidableTotalFlora = computeTotalFlora (m_nonCollidableMaximumDistance, m_nonCollidableMinimumDistance, m_nonCollidableTileSize);
	m_radialTotalFlora        = computeTotalFlora (m_radialMaximumDistance, m_radialMinimumDistance, m_radialTileSize);
	m_farRadialTotalFlora     = computeTotalFlora (m_farRadialMaximumDistance, m_farRadialMinimumDistance, m_farRadialTileSize);

	UpdateData (false);
}

//===================================================================

