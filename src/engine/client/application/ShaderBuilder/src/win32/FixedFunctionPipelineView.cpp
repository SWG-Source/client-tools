// FixedFunctionPipelineView.cpp : implementation file
//

#include "FirstShaderBuilder.h"
#include "shaderbuilder.h"
#include "FixedFunctionPipelineView.h"

#include "Node.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFixedFunctionPipelineView

IMPLEMENT_DYNCREATE(CFixedFunctionPipelineView, CFormView)

BEGIN_MESSAGE_MAP(CFixedFunctionPipelineView, CFormView)
	//{{AFX_MSG_MAP(CFixedFunctionPipelineView)
	ON_BN_CLICKED(IDC_PASS_LIGHTING_CHECK, OnLighting)
	ON_BN_CLICKED(IDC_PASS_SPECULARENABLE_CHECK, OnSpecularenable)
	ON_BN_CLICKED(IDC_PASS_COLORVERTEX_CHECK, OnColorvertex)
	ON_CBN_SELCHANGE(IDC_PASS_ABMIENTCOLORSOURCE_COMBO, OnSelchangeAmbientmaterialsource)
	ON_CBN_SELCHANGE(IDC_PASS_DIFFUSECOLORSOURCE_COMBO, OnSelchangeDiffusematerialsource)
	ON_CBN_SELCHANGE(IDC_PASS_SPECULARCOLORSOURCE_COMBO, OnSelchangeSpecularmaterialsource)
	ON_CBN_SELCHANGE(IDC_PASS_EMISSIVECOLORSOURCE_COMBO, OnSelchangeEmissivematerialsource)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CFixedFunctionPipelineView::CFixedFunctionPipelineView()
	: CFormView(CFixedFunctionPipelineView::IDD)
{
	//{{AFX_DATA_INIT(CFixedFunctionPipelineView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CFixedFunctionPipelineView::~CFixedFunctionPipelineView()
{
}

void CFixedFunctionPipelineView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFixedFunctionPipelineView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_PASS_EMISSIVECOLORSOURCE_COMBO, m_lightingEmissiveColorSource);
	DDX_Control(pDX, IDC_PASS_SPECULARCOLORSOURCE_COMBO, m_lightingSpecularColorSource);
	DDX_Control(pDX, IDC_PASS_DIFFUSECOLORSOURCE_COMBO, m_lightingDiffuseColorSource);
	DDX_Control(pDX, IDC_PASS_ABMIENTCOLORSOURCE_COMBO, m_lightingAmbientColorSource);
	DDX_Control(pDX, IDC_PASS_COLORVERTEX_CHECK, m_lightingColorVertex);
	DDX_Control(pDX, IDC_PASS_SPECULARENABLE_CHECK, m_lightingSpecularEnable);
	DDX_Control(pDX, IDC_PASS_LIGHTING_CHECK, m_lighting);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CFixedFunctionPipelineView diagnostics

#ifdef _DEBUG
void CFixedFunctionPipelineView::AssertValid() const
{
	CFormView::AssertValid();
}

void CFixedFunctionPipelineView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFixedFunctionPipelineView message handlers

void CFixedFunctionPipelineView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();

}

// ----------------------------------------------------------------------

void CFixedFunctionPipelineView::SetNode(CFixedFunctionPipelineNode &node)
{
	fixedFunctionPipelineNode = &node;

#define SET(a) a.SetCheck(node.a)

	SET(m_lightingColorVertex);
	SET(m_lightingSpecularEnable);
	SET(m_lighting);

#undef SET
#define SET(a) do { if (node.a == "") a.SetCurSel(-1); else a.SelectString(-1, node.a); } while (0)

	SET(m_lightingEmissiveColorSource);
	SET(m_lightingSpecularColorSource);
	SET(m_lightingDiffuseColorSource);
	SET(m_lightingAmbientColorSource);

	if (fixedFunctionPipelineNode->IsLocked(CNode::Either))	
		EnableWindow(FALSE);
	else
		EnableWindow(TRUE);
}

// ----------------------------------------------------------------------

void CFixedFunctionPipelineView::OnLighting() 
{
	fixedFunctionPipelineNode->m_lighting = m_lighting.GetCheck();
}

#if 0

#endif

// ----------------------------------------------------------------------

void CFixedFunctionPipelineView::OnSpecularenable() 
{
	fixedFunctionPipelineNode->m_lightingSpecularEnable = m_lightingSpecularEnable.GetCheck();
}

// ----------------------------------------------------------------------

void CFixedFunctionPipelineView::OnColorvertex() 
{	
	fixedFunctionPipelineNode->m_lightingColorVertex = m_lightingColorVertex.GetCheck();
}


// ----------------------------------------------------------------------

void CFixedFunctionPipelineView::OnSelchangeAmbientmaterialsource() 
{
	m_lightingAmbientColorSource.GetWindowText(fixedFunctionPipelineNode->m_lightingAmbientColorSource);
}

// ----------------------------------------------------------------------

void CFixedFunctionPipelineView::OnSelchangeDiffusematerialsource() 
{
	m_lightingDiffuseColorSource.GetWindowText(fixedFunctionPipelineNode->m_lightingDiffuseColorSource);
}

// ----------------------------------------------------------------------

void CFixedFunctionPipelineView::OnSelchangeSpecularmaterialsource() 
{
	m_lightingSpecularColorSource.GetWindowText(fixedFunctionPipelineNode->m_lightingSpecularColorSource);
}

// ----------------------------------------------------------------------

void CFixedFunctionPipelineView::OnSelchangeEmissivematerialsource() 
{
	m_lightingEmissiveColorSource.GetWindowText(fixedFunctionPipelineNode->m_lightingEmissiveColorSource);
}

