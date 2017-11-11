// TextureScrollView.cpp : implementation file
//

#include "FirstShaderBuilder.h"
#include "shaderbuilder.h"
#include "TextureScrollView.h"

#include "Node.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextureScrollView

IMPLEMENT_DYNCREATE(CTextureScrollView, CFormView)

BEGIN_MESSAGE_MAP(CTextureScrollView, CFormView)
	//{{AFX_MSG_MAP(CTextureScrollView)
	ON_EN_CHANGE(IDC_PASS_TEXTURESCROLL1U, OnChangeTextureScroll1U)
	ON_EN_CHANGE(IDC_PASS_TEXTURESCROLL1V, OnChangeTextureScroll1V)
	ON_EN_CHANGE(IDC_PASS_TEXTURESCROLL2U, OnChangeTextureScroll2U)
	ON_EN_CHANGE(IDC_PASS_TEXTURESCROLL2V, OnChangeTextureScroll2V)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CTextureScrollView::CTextureScrollView()
	: CFormView(CTextureScrollView::IDD)
{
	//{{AFX_DATA_INIT(CTextureScrollView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CTextureScrollView::~CTextureScrollView()
{
}

void CTextureScrollView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTextureScrollView)
	DDX_Control(pDX, IDC_PASS_TEXTURESCROLL1U, m_textureScroll1U);
	DDX_Control(pDX, IDC_PASS_TEXTURESCROLL1V, m_textureScroll1V);
	DDX_Control(pDX, IDC_PASS_TEXTURESCROLL2U, m_textureScroll2U);
	DDX_Control(pDX, IDC_PASS_TEXTURESCROLL2V, m_textureScroll2V);
	//}}AFX_DATA_MAP
}

void CTextureScrollView::SetNode(CTextureScrollNode &node)
{
	m_scrollNode = &node;

#undef SET
#define SET(a) a.SetWindowText(node.a)

	SET(m_textureScroll1U);
	SET(m_textureScroll1V);
	SET(m_textureScroll2U);
	SET(m_textureScroll2V);
}

/////////////////////////////////////////////////////////////////////////////
// CTextureScrollView diagnostics

#ifdef _DEBUG
void CTextureScrollView::AssertValid() const
{
	CFormView::AssertValid();
}

void CTextureScrollView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTextureScrollView message handlers


void CTextureScrollView::OnChangeTextureScroll1U()
{
	m_textureScroll1U.GetWindowText(m_scrollNode->m_textureScroll1U);
}

void CTextureScrollView::OnChangeTextureScroll1V()
{
	m_textureScroll1V.GetWindowText(m_scrollNode->m_textureScroll1V);
}

void CTextureScrollView::OnChangeTextureScroll2U()
{
	m_textureScroll2U.GetWindowText(m_scrollNode->m_textureScroll2U);
}

void CTextureScrollView::OnChangeTextureScroll2V()
{
	m_textureScroll2V.GetWindowText(m_scrollNode->m_textureScroll2V);
}
