// VertexShaderView.cpp : implementation file
//

#include "FirstShaderBuilder.h"
#include "shaderbuilder.h"
#include "VertexShaderView.h"

#include "sharedFile/TreeFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVertexShaderView

IMPLEMENT_DYNCREATE(CVertexShaderView, CFormView)

BEGIN_MESSAGE_MAP(CVertexShaderView, CFormView)
	//{{AFX_MSG_MAP(CVertexShaderView)
	ON_EN_CHANGE(IDC_PIXELSHADER_TEXT_EDIT, OnChangePixelshaderTextEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CVertexShaderView::CVertexShaderView()
	: CFormView(CVertexShaderView::IDD),
	m_vertexShaderNode(NULL)
{
	//{{AFX_DATA_INIT(CVertexShaderView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CVertexShaderView::~CVertexShaderView()
{
}

void CVertexShaderView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVertexShaderView)
	DDX_Control(pDX, IDC_PIXELSHADER_TEXT_EDIT, m_fileName);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CVertexShaderView diagnostics

#ifdef _DEBUG
void CVertexShaderView::AssertValid() const
{
	CFormView::AssertValid();
}

void CVertexShaderView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CVertexShaderView message handlers

void CVertexShaderView::SetNode(CVertexShaderNode &node)
{
	m_vertexShaderNode = &node;

	if (m_vertexShaderNode->IsLocked(CNode::Either))	
		EnableWindow(FALSE);
	else
		EnableWindow(TRUE);

	m_fileName.SetWindowText(m_vertexShaderNode->m_fileName);
}

void CVertexShaderView::OnChangePixelshaderTextEdit() 
{
	CString s;
	m_fileName.GetWindowText(s);

	// clean up the text input
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

	if (m_vertexShaderNode->m_fileName != s)
	{
		CVertexShaderProgramTreeNode &treeNode = CTemplateTreeNode::GetInstance()->GetVertexShaderProgramTree();
		
		treeNode.Remove(m_vertexShaderNode->m_fileName, false);
		m_vertexShaderNode->m_fileName = s;
		treeNode.Add(s, false);
	}
}
