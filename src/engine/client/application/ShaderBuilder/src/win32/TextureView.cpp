// TextureView.cpp : implementation file
//

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"
#include "TextureView.h"

#include "Node.h"
#include "RecentDirectory.h"
#include "sharedFile/TreeFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextureView

IMPLEMENT_DYNCREATE(CTextureView, CFormView)

CTextureView::CTextureView()
	: CFormView(CTextureView::IDD),
	textureNode(0)
{
	//{{AFX_DATA_INIT(CTextureView)
	//}}AFX_DATA_INIT
}

CTextureView::~CTextureView()
{
}

void CTextureView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	EnableToolTips(true);   // enable tool tips for view
}

void CTextureView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTextureView)
	DDX_Control(pDX, IDC_TEXTURE_ANISOTROPY, m_anisotropy);
	DDX_Control(pDX, IDC_TEXTURE_FILTERMIP_COMBO, m_filterMip);
	DDX_Control(pDX, IDC_TEXTURE_FILTERMIN_COMBO, m_filterMin);
	DDX_Control(pDX, IDC_TEXTURE_FILTERMAG_COMBO, m_filterMag);
	DDX_Control(pDX, IDC_TEXTURE_ADDRESSW_COMBO, m_addressW);
	DDX_Control(pDX, IDC_TEXTURE_ADDRESSV_COMBO, m_addressV);
	DDX_Control(pDX, IDC_TEXTURE_ADDRESSU_COMBO, m_addressU);
	DDX_Control(pDX, IDC_TEXTURE_PLACEHOLDER_CHECK, m_placeHolder);
	DDX_Control(pDX, IDC_TEXTURE_TEXTURENAME_EDIT, m_textureName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTextureView, CFormView)
	//{{AFX_MSG_MAP(CTextureView)
	ON_EN_CHANGE(IDC_TEXTURE_TEXTURENAME_EDIT, OnChangeTexturefilename)
	ON_EN_KILLFOCUS(IDC_TEXTURE_TEXTURENAME_EDIT, OnKillfocusTexturefilename)
	ON_BN_CLICKED(IDC_TEXTURE_PLACEHOLDER_CHECK, OnCheckPlaceholder)
	ON_BN_CLICKED(IDC_TEXTURE_BROWSE_BUTTON, OnTextureBrowse)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
	ON_CBN_SELCHANGE(IDC_TEXTURE_ADDRESSU_COMBO, OnChangeAddressU)
	ON_CBN_SELCHANGE(IDC_TEXTURE_ADDRESSV_COMBO, OnChangeAddressV)
	ON_CBN_SELCHANGE(IDC_TEXTURE_ADDRESSW_COMBO, OnChangeAddressW)
	ON_CBN_SELCHANGE(IDC_TEXTURE_FILTERMAG_COMBO, OnChangeFilterMag)
	ON_CBN_SELCHANGE(IDC_TEXTURE_FILTERMIN_COMBO, OnChangeFilterMin)
	ON_CBN_SELCHANGE(IDC_TEXTURE_FILTERMIP_COMBO, OnChangeFilterMip)
	ON_CBN_SELCHANGE(IDC_TEXTURE_ANISOTROPY, OnChangeAnisotropy)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextureView diagnostics

#ifdef _DEBUG
void CTextureView::AssertValid() const
{
	CFormView::AssertValid();
}

void CTextureView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTextureView message handlers

// Set a combo box's value to the string in the node.

static void stop()
{
	__asm int 3;
}

#define SET(a) \
do \
{ \
	if (node.a == "") \
		a.SetCurSel(-1); \
	else \
		a.SelectString(-1, node.a); \
	CString value; \
	a.GetWindowText(value); \
	if (node.a != value) \
		stop(); \
} while (0)

// ----------

void CTextureView::SetNode(CTextureNode &node)
{
	textureNode = &node;

	SET(m_addressU);
	SET(m_addressV);
	SET(m_addressW);

	SET(m_filterMag);
	SET(m_filterMin);
	SET(m_filterMip);
	
	if (node.m_anisotropy == 255)
		m_anisotropy.SetCurSel(9);
	else
		m_anisotropy.SetCurSel(node.m_anisotropy);

	m_placeHolder.SetCheck(node.m_placeHolder);
	m_textureName.SetWindowText(node.m_textureName);
}

// ----------

#undef SET

// ----------------------------------------------------------------------

void CTextureView::OnChangeTexturefilename() 
{
	CString s;
	m_textureName.GetWindowText(s);

	// clean up the text input
	char *buffer = new char[s.GetLength() + 1];
	TreeFile::fixUpFileName(s, buffer);
	if (strcmp(s, buffer) != 0)
	{
		s = buffer;

		int start, end;
		m_textureName.GetSel(start, end);
		m_textureName.SetWindowText(s);
		m_textureName.SetSel(start, end);
	}
	delete [] buffer;

	textureNode->m_textureName = s;
}

void CTextureView::OnCheckPlaceholder() 
{
	textureNode->m_placeHolder = m_placeHolder.GetCheck();	
}

void CTextureView::OnTextureBrowse() 
{
	// Open a browse dialog
	CFileDialog fileDialog(TRUE, "*.dds", 0, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, "Texture *.dds|*.dds||");
	fileDialog.m_ofn.Flags |= OFN_FILEMUSTEXIST;
	fileDialog.m_ofn.lpstrInitialDir = RecentDirectory::find("Texture");
	fileDialog.m_ofn.lpstrTitle      = "Load Texture";

	// Make sure the user selected OK
	if (fileDialog.DoModal() == IDOK)
	{
		CString fileName = fileDialog.GetPathName();
		RecentDirectory::update("Texture", fileName);
		std::string pathedFileName = static_cast<CShaderBuilderApp*>(AfxGetApp())->stripTreefileSearchPathFromFile(static_cast<LPCSTR>(fileName));
		m_textureName.SetWindowText(pathedFileName.c_str());
		//if the final filename has a : in it, warn about possible absolute path
		if(pathedFileName.find(":", 0) != std::string::npos)
		{
			MessageBox("Absolute pathed texture detected.  File might not be in TreeFile search path.", "Warning", MB_OK);
		}
	}
}

void CTextureView::OnKillfocusTexturefilename() 
{
	CString name;
	m_textureName.GetWindowText(name);

	std::string pathedFileName = static_cast<CShaderBuilderApp*>(AfxGetApp())->stripTreefileSearchPathFromFile(static_cast<LPCSTR>(name));
	m_textureName.SetWindowText(pathedFileName.c_str());
}

BOOL CTextureView::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
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

// ----------------------------------------------------------------------

#define UPDATE_NODE(A) {A.GetWindowText(textureNode->A);}

// ----------

void	CTextureView::OnChangeAddressU		( void ) { UPDATE_NODE(m_addressU); }
void	CTextureView::OnChangeAddressV		( void ) { UPDATE_NODE(m_addressV); }
void	CTextureView::OnChangeAddressW		( void ) { UPDATE_NODE(m_addressW); }

void	CTextureView::OnChangeFilterMag		( void ) { UPDATE_NODE(m_filterMag); }
void	CTextureView::OnChangeFilterMin		( void ) { UPDATE_NODE(m_filterMin); }
void	CTextureView::OnChangeFilterMip		( void ) { UPDATE_NODE(m_filterMip); }

// ----------

#undef UPDATE_NODE

void	CTextureView::OnChangeAnisotropy()
{
	int sel = m_anisotropy.GetCurSel();
	if (sel == 9)
		sel = 255;

	textureNode->m_anisotropy = sel;
}

// ----------------------------------------------------------------------
