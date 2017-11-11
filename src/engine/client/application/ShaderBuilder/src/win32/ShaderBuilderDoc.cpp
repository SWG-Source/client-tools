// ShaderBuilderDoc.cpp : implementation of the CShaderBuilderDoc class
//

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"

#include "sharedFoundation/ExitChain.h"
#include "ShaderBuilderDoc.h"
#include "LeftView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CLeftView *CShaderBuilderDoc::leftView;

/////////////////////////////////////////////////////////////////////////////
// CShaderBuilderDoc

IMPLEMENT_DYNCREATE(CShaderBuilderDoc, CDocument)

BEGIN_MESSAGE_MAP(CShaderBuilderDoc, CDocument)
	//{{AFX_MSG_MAP(CShaderBuilderDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShaderBuilderDoc construction/destruction

CShaderBuilderDoc::CShaderBuilderDoc()
{
	// TODO: add one-time construction code here

}

CShaderBuilderDoc::~CShaderBuilderDoc()
{
	ExitChain::quit();
}

BOOL CShaderBuilderDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	leftView->ResetTree();

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CShaderBuilderDoc serialization

void CShaderBuilderDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CShaderBuilderDoc diagnostics

#ifdef _DEBUG
void CShaderBuilderDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CShaderBuilderDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CShaderBuilderDoc commands

BOOL CShaderBuilderDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	// TODO: Add your specialized creation code here	
	return leftView->LoadTemplate(lpszPathName);
}

BOOL CShaderBuilderDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	UNREF(lpszPathName);
	// TODO: Add your specialized code here and/or call the base class
	__asm int 3;
	// return 	leftView->Save(lpszPathName);
	return FALSE;
}
