// PixelShaderView.cpp : implementation file
//

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"
#include "PixelShaderView.h"

#include "sharedFile/TreeFile.h"

#include "Node.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPixelShaderView

IMPLEMENT_DYNCREATE(CPixelShaderView, CFormView)

CPixelShaderView::CPixelShaderView()
	: CFormView(CPixelShaderView::IDD),
	m_pixelShaderNode(NULL)
{
	//{{AFX_DATA_INIT(CPixelShaderView)
	//}}AFX_DATA_INIT
}

CPixelShaderView::~CPixelShaderView()
{
}

void CPixelShaderView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPixelShaderView)
	DDX_Control(pDX, IDC_PIXELSHADER_TEXT_EDIT, m_fileName);
	DDX_Control(pDX, IDC_PIXELSHADER_T0, m_t0);
	DDX_Control(pDX, IDC_PIXELSHADER_T1, m_t1);
	DDX_Control(pDX, IDC_PIXELSHADER_T2, m_t2);
	DDX_Control(pDX, IDC_PIXELSHADER_T3, m_t3);
	DDX_Control(pDX, IDC_PIXELSHADER_T4, m_t4);
	DDX_Control(pDX, IDC_PIXELSHADER_T5, m_t5);
	DDX_Control(pDX, IDC_PIXELSHADER_T6, m_t6);
	DDX_Control(pDX, IDC_PIXELSHADER_T7, m_t7);
	DDX_Control(pDX, IDC_PIXELSHADER_T8, m_t8);
	DDX_Control(pDX, IDC_PIXELSHADER_T9, m_t9);
	DDX_Control(pDX, IDC_PIXELSHADER_T10, m_t10);
	DDX_Control(pDX, IDC_PIXELSHADER_T11, m_t11);
	DDX_Control(pDX, IDC_PIXELSHADER_T12, m_t12);
	DDX_Control(pDX, IDC_PIXELSHADER_T13, m_t13);
	DDX_Control(pDX, IDC_PIXELSHADER_T14, m_t14);
	DDX_Control(pDX, IDC_PIXELSHADER_T15, m_t15);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPixelShaderView, CFormView)
	//{{AFX_MSG_MAP(CPixelShaderView)
	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_PIXELSHADER_TEXT_EDIT, OnChangeEdit2)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
	ON_EN_CHANGE(IDC_PIXELSHADER_T0, OnChangeT0)
	ON_EN_CHANGE(IDC_PIXELSHADER_T1, OnChangeT1)
	ON_EN_CHANGE(IDC_PIXELSHADER_T2, OnChangeT2)
	ON_EN_CHANGE(IDC_PIXELSHADER_T3, OnChangeT3)
	ON_EN_CHANGE(IDC_PIXELSHADER_T4, OnChangeT4)
	ON_EN_CHANGE(IDC_PIXELSHADER_T5, OnChangeT5)
	ON_EN_CHANGE(IDC_PIXELSHADER_T6, OnChangeT6)
	ON_EN_CHANGE(IDC_PIXELSHADER_T7, OnChangeT7)
	ON_EN_CHANGE(IDC_PIXELSHADER_T8, OnChangeT8)
	ON_EN_CHANGE(IDC_PIXELSHADER_T9, OnChangeT9)
	ON_EN_CHANGE(IDC_PIXELSHADER_T10, OnChangeT10)
	ON_EN_CHANGE(IDC_PIXELSHADER_T11, OnChangeT11)
	ON_EN_CHANGE(IDC_PIXELSHADER_T12, OnChangeT12)
	ON_EN_CHANGE(IDC_PIXELSHADER_T13, OnChangeT13)
	ON_EN_CHANGE(IDC_PIXELSHADER_T14, OnChangeT14)
	ON_EN_CHANGE(IDC_PIXELSHADER_T15, OnChangeT15)
	ON_EN_KILLFOCUS(IDC_PIXELSHADER_T0, OnKillfocusT0)
	ON_EN_KILLFOCUS(IDC_PIXELSHADER_T1, OnKillfocusT1)
	ON_EN_KILLFOCUS(IDC_PIXELSHADER_T2, OnKillfocusT2)
	ON_EN_KILLFOCUS(IDC_PIXELSHADER_T3, OnKillfocusT3)
	ON_EN_KILLFOCUS(IDC_PIXELSHADER_T4, OnKillfocusT4)
	ON_EN_KILLFOCUS(IDC_PIXELSHADER_T5, OnKillfocusT5)
	ON_EN_KILLFOCUS(IDC_PIXELSHADER_T6, OnKillfocusT6)
	ON_EN_KILLFOCUS(IDC_PIXELSHADER_T7, OnKillfocusT7)
	ON_EN_KILLFOCUS(IDC_PIXELSHADER_T8, OnKillfocusT8)
	ON_EN_KILLFOCUS(IDC_PIXELSHADER_T9, OnKillfocusT9)
	ON_EN_KILLFOCUS(IDC_PIXELSHADER_T10, OnKillfocusT10)
	ON_EN_KILLFOCUS(IDC_PIXELSHADER_T11, OnKillfocusT11)
	ON_EN_KILLFOCUS(IDC_PIXELSHADER_T12, OnKillfocusT12)
	ON_EN_KILLFOCUS(IDC_PIXELSHADER_T13, OnKillfocusT13)
	ON_EN_KILLFOCUS(IDC_PIXELSHADER_T14, OnKillfocusT14)
	ON_EN_KILLFOCUS(IDC_PIXELSHADER_T15, OnKillfocusT15)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPixelShaderView diagnostics

#ifdef _DEBUG
void CPixelShaderView::AssertValid() const
{
	CFormView::AssertValid();
}

void CPixelShaderView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPixelShaderView message handlers


void CPixelShaderView::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	CFormView::ShowScrollBar(SB_VERT,FALSE);
	CFormView::ShowScrollBar(SB_HORZ,FALSE);
}

void CPixelShaderView::SetNode(CPixelShaderNode &pixelShaderNode)
{
	m_pixelShaderNode = &pixelShaderNode;
	m_fileName.SetWindowText(m_pixelShaderNode->m_fileName);

#define SET(a) a.SetWindowText(pixelShaderNode.a)

	SET(m_t0);
	SET(m_t1);
	SET(m_t2);
	SET(m_t3);
	SET(m_t4);
	SET(m_t5);
	SET(m_t6);
	SET(m_t7);
	SET(m_t8);
	SET(m_t9);
	SET(m_t10);
	SET(m_t11);
	SET(m_t12);
	SET(m_t13);
	SET(m_t14);
	SET(m_t15);

#undef set

	if (m_pixelShaderNode->IsLocked(CNode::Either))	
		EnableWindow(FALSE);
	else
		EnableWindow(TRUE);
}

void CPixelShaderView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	EnableToolTips(true);   // enable tool tips for view
	m_t0.SetLimitText(4);
	m_t1.SetLimitText(4);
	m_t2.SetLimitText(4);
	m_t3.SetLimitText(4);
	m_t4.SetLimitText(4);
	m_t5.SetLimitText(4);
	m_t6.SetLimitText(4);
	m_t7.SetLimitText(4);
	m_t8.SetLimitText(4);
	m_t9.SetLimitText(4);
	m_t10.SetLimitText(4);
	m_t11.SetLimitText(4);
	m_t12.SetLimitText(4);
	m_t13.SetLimitText(4);
	m_t14.SetLimitText(4);
	m_t15.SetLimitText(4);
}

void CPixelShaderView::OnChangeEdit2() 
{
	CString s;
	m_fileName.GetWindowText(s);

	char *buffer = new char[s.GetLength() + 1];
	TreeFile::fixUpFileName(s, buffer);
	
	if (strcmp(s, buffer) != 0)
	{
		s = buffer;

		int start, end;
		m_fileName.GetSel(start, end);
		m_fileName.SetWindowText(s);
		m_fileName.SetSel(start, end);
	}
	delete [] buffer;

	if (m_pixelShaderNode->m_fileName != s)
	{
		CPixelShaderProgramTreeNode &treeNode = CTemplateTreeNode::GetInstance()->GetPixelShaderProgramTree();
		
		treeNode.Remove(m_pixelShaderNode->m_fileName, false);
		m_pixelShaderNode->m_fileName = s;
		treeNode.Add(s, false);
	}
}

BOOL CPixelShaderView::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
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

#define GenerateFunctions(T) \
	void CPixelShaderView::OnChangeT ## T() \
	{ \
		CString s; \
		m_t ## T.GetWindowText(s); \
		UpdateTaggedTree(CTemplateTreeNode::GetInstance()->GetTextureTree(), m_pixelShaderNode->m_t ## T, s); \
	} \
	void CPixelShaderView::OnKillfocusT ## T() \
	{ \
		Pad4(m_t ## T); \
	}

GenerateFunctions(0)
GenerateFunctions(1)
GenerateFunctions(2)
GenerateFunctions(3)
GenerateFunctions(4)
GenerateFunctions(5)
GenerateFunctions(6)
GenerateFunctions(7)
GenerateFunctions(8)
GenerateFunctions(9)
GenerateFunctions(10)
GenerateFunctions(11)
GenerateFunctions(12)
GenerateFunctions(13)
GenerateFunctions(14)
GenerateFunctions(15)
