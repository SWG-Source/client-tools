// ShaderBuilderDoc.h : interface of the CShaderBuilderDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHADERBUILDERDOC_H__7E03F8CA_3224_4D52_BA46_304F6F4FAB20__INCLUDED_)
#define AFX_SHADERBUILDERDOC_H__7E03F8CA_3224_4D52_BA46_304F6F4FAB20__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLeftView;

class CShaderBuilderDoc : public CDocument
{
protected: // create from serialization only
	CShaderBuilderDoc();
	DECLARE_DYNCREATE(CShaderBuilderDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShaderBuilderDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CShaderBuilderDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	static CLeftView *leftView;

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CShaderBuilderDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHADERBUILDERDOC_H__7E03F8CA_3224_4D52_BA46_304F6F4FAB20__INCLUDED_)
