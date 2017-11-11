// MaterialView.cpp : implementation file
//

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"
#include "MaterialView.h"

#include "Node.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMaterialView

IMPLEMENT_DYNCREATE(CMaterialView, CFormView)

CMaterialView::CMaterialView()
	: CFormView(CMaterialView::IDD),
	materialNode(0)
{
	//{{AFX_DATA_INIT(CMaterialView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CMaterialView::~CMaterialView()
{
}

void CMaterialView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMaterialView)
	DDX_Control(pDX, IDC_MATERIAL_DIFFUSERED_EDIT, m_diffuseRed);
	DDX_Control(pDX, IDC_MATERIAL_DIFFUSEGREEN_EDIT, m_diffuseGreen);
	DDX_Control(pDX, IDC_MATERIAL_DIFFUSEBLUE_EDIT, m_diffuseBlue);
	DDX_Control(pDX, IDC_MATERIAL_DIFFUSEALPHA_EDIT, m_diffuseAlpha);
	DDX_Control(pDX, IDC_MATERIAL_AMBIENTRED_EDIT, m_ambientRed);
	DDX_Control(pDX, IDC_MATERIAL_AMBIENTGREEN_EDIT, m_ambientGreen);
	DDX_Control(pDX, IDC_MATERIAL_AMBIENTBLUE_EDIT, m_ambientBlue);
	DDX_Control(pDX, IDC_MATERIAL_AMBIENTALPHA_EDIT, m_ambientAlpha);
	DDX_Control(pDX, IDC_MATERIAL_SPECULARRED_EDIT, m_specularRed);
	DDX_Control(pDX, IDC_MATERIAL_SPECULARGREEN_EDIT, m_specularGreen);
	DDX_Control(pDX, IDC_MATERIAL_SPECULARBLUE_EDIT, m_specularBlue);
	DDX_Control(pDX, IDC_MATERIAL_SPECULARALPHA_EDIT, m_specularAlpha);
	DDX_Control(pDX, IDC_MATERIAL_EMISSIVERED_EDIT, m_emissiveRed);
	DDX_Control(pDX, IDC_MATERIAL_EMISSIVEGREEN_EDIT, m_emissiveGreen);
	DDX_Control(pDX, IDC_MATERIAL_EMISSIVEBLUE_EDIT, m_emissiveBlue);
	DDX_Control(pDX, IDC_MATERIAL_EMISSIVEALPHA_EDIT, m_emissiveAlpha);
	DDX_Control(pDX, IDC_MATERIAL_POWER_EDIT, m_power);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMaterialView, CFormView)
	//{{AFX_MSG_MAP(CMaterialView)
	ON_EN_CHANGE(IDC_MATERIAL_DIFFUSERED_EDIT, OnChangeDiffuseRed)
	ON_EN_CHANGE(IDC_MATERIAL_DIFFUSEGREEN_EDIT, OnChangeDiffuseGreen)
	ON_EN_CHANGE(IDC_MATERIAL_DIFFUSEBLUE_EDIT, OnChangeDiffuseblue)
	ON_EN_CHANGE(IDC_MATERIAL_DIFFUSEALPHA_EDIT, OnChangeDiffusealpha)
	ON_EN_CHANGE(IDC_MATERIAL_AMBIENTRED_EDIT, OnChangeAmbientred)
	ON_EN_CHANGE(IDC_MATERIAL_AMBIENTGREEN_EDIT, OnChangeAmbientgreen)
	ON_EN_CHANGE(IDC_MATERIAL_AMBIENTBLUE_EDIT, OnChangeAmbientblue)
	ON_EN_CHANGE(IDC_MATERIAL_AMBIENTALPHA_EDIT, OnChangeAmbientalpha)
	ON_EN_CHANGE(IDC_MATERIAL_SPECULARRED_EDIT, OnChangeSpecularred)
	ON_EN_CHANGE(IDC_MATERIAL_SPECULARGREEN_EDIT, OnChangeSpeculargreen)
	ON_EN_CHANGE(IDC_MATERIAL_SPECULARBLUE_EDIT, OnChangeSpecularblue)
	ON_EN_CHANGE(IDC_MATERIAL_SPECULARALPHA_EDIT, OnChangeSpecularAlpha)
	ON_EN_CHANGE(IDC_MATERIAL_EMISSIVERED_EDIT, OnChangeEmissivered)
	ON_EN_CHANGE(IDC_MATERIAL_EMISSIVEGREEN_EDIT, OnChangeEmissivegreen)
	ON_EN_CHANGE(IDC_MATERIAL_EMISSIVEBLUE_EDIT, OnChangeEmissiveblue)
	ON_EN_CHANGE(IDC_MATERIAL_EMISSIVEALPHA_EDIT, OnChangeEmissivealpha)
	ON_EN_CHANGE(IDC_MATERIAL_POWER_EDIT, OnChangePower)
	ON_BN_CLICKED(IDC_MATERIAL_PICKDIFFUSE_BUTTON, OnButton1)
	ON_BN_CLICKED(IDC_MATERIAL_PICKAMBIENT_BUTTON, OnButton2)
	ON_BN_CLICKED(IDC_MATERIAL_PICKSPECULAR_BUTTON, OnButton3)
	ON_BN_CLICKED(IDC_MATERIAL_PICKEMISSIVE_BUTTON, OnButton4)
	ON_WM_HELPINFO()
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMaterialView diagnostics

#ifdef _DEBUG
void CMaterialView::AssertValid() const
{
	CFormView::AssertValid();
}

void CMaterialView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMaterialView message handlers

void CMaterialView::SetNode(CMaterialNode &node)
{
	materialNode = &node;

#define SET(a)  a.SetWindowText(node.a)

	SET(m_power);
	SET(m_emissiveBlue);
	SET(m_emissiveGreen);
	SET(m_emissiveRed);
	SET(m_emissiveAlpha);
	SET(m_specularBlue);
	SET(m_specularGreen);
	SET(m_specularRed);
	SET(m_specularAlpha);
	SET(m_ambientBlue);
	SET(m_ambientGreen);
	SET(m_ambientRed);
	SET(m_ambientAlpha);
	SET(m_diffuseBlue);
	SET(m_diffuseGreen);
	SET(m_diffuseRed);
	SET(m_diffuseAlpha);
	
#undef SET
}

#define COPY(a) a.GetWindowText(materialNode->a)

void CMaterialView::OnChangeDiffusealpha() 
{
	COPY(m_diffuseAlpha);	
}

void CMaterialView::OnChangeDiffuseRed() 
{
	COPY(m_diffuseRed);	
}

void CMaterialView::OnChangeDiffuseGreen() 
{
	COPY(m_diffuseGreen);
}

void CMaterialView::OnChangeDiffuseblue() 
{
	COPY(m_diffuseBlue);	
}

void CMaterialView::OnChangeAmbientalpha() 
{
	COPY(m_ambientAlpha);	
}

void CMaterialView::OnChangeAmbientred() 
{
	COPY(m_ambientRed);	
}

void CMaterialView::OnChangeAmbientgreen() 
{
	COPY(m_ambientGreen);	
}

void CMaterialView::OnChangeAmbientblue() 
{
	COPY(m_ambientBlue);	
}

void CMaterialView::OnChangeSpecularAlpha() 
{
	COPY(m_specularAlpha);	
}

void CMaterialView::OnChangeSpecularred() 
{
	COPY(m_specularRed);	
}

void CMaterialView::OnChangeSpeculargreen() 
{
	COPY(m_specularGreen);	
}

void CMaterialView::OnChangeSpecularblue() 
{
	COPY(m_specularBlue);	
}

void CMaterialView::OnChangeEmissivealpha() 
{
	COPY(m_emissiveAlpha);	
}

void CMaterialView::OnChangeEmissivered() 
{
	COPY(m_emissiveRed);	
}

void CMaterialView::OnChangeEmissivegreen() 
{
	COPY(m_emissiveGreen);	
}

void CMaterialView::OnChangeEmissiveblue() 
{
	COPY(m_emissiveBlue);	
}

void CMaterialView::OnChangePower() 
{
	COPY(m_power);	
}

void CMaterialView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	EnableToolTips(true);   // enable tool tips for view
}

BOOL CMaterialView::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	// TODO: Add your message handler code here and/or call default
	
	return CFormView::OnHelpInfo(pHelpInfo);
}

void CMaterialView::OnButton1() 
{
	ColorPick(m_diffuseRed, m_diffuseGreen, m_diffuseBlue);
}

void CMaterialView::OnButton2() 
{
	ColorPick(m_ambientRed, m_ambientGreen, m_ambientBlue);
}

void CMaterialView::OnButton3() 
{
	ColorPick(m_specularRed, m_specularGreen, m_specularBlue);
}

void CMaterialView::OnButton4() 
{
	ColorPick(m_emissiveRed, m_emissiveGreen, m_emissiveBlue);
}

void CMaterialView::ColorPick(CEdit &red, CEdit &green, CEdit &blue)
{
	CColorDialog dialog;

	if (dialog.DoModal() == IDOK)
	{
		COLORREF colorRef = dialog.GetColor();
		const int r = GetRValue(colorRef);
		const int g = GetGValue(colorRef);
		const int b = GetBValue(colorRef);

		char rt[8];
		sprintf(rt, "%1.5f", static_cast<float>(r) / 255.0f);

		char gt[8];
		sprintf(gt, "%1.5f", static_cast<float>(g) / 255.0f);

		char bt[8];
		sprintf(bt, "%1.5f", static_cast<float>(b) / 255.0f);

		red.SetWindowText(rt);
		green.SetWindowText(gt);
		blue.SetWindowText(bt);
	}
}

BOOL CMaterialView::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREF(pResult);
	UNREF(id);
	TOOLTIPTEXT *pTTT = reinterpret_cast<TOOLTIPTEXT *>(pNMHDR);
	UINT nID = pNMHDR->idFrom;
	if (pTTT->uFlags & TTF_IDISHWND)
	{
		// idFrom is actually the HWND of the tool
		nID = ::GetDlgCtrlID((HWND)nID);
		if(nID)
		{
			pTTT->lpszText = MAKEINTRESOURCE(nID);
			pTTT->hinst = AfxGetResourceHandle();
			return TRUE;
		}
	}
	return FALSE;
}

