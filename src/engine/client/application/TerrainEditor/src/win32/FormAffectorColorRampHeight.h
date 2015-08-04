//
// FormAffectorColorRampHeight.h
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormAffectorColorRampHeight_H
#define INCLUDED_FormAffectorColorRampHeight_H

//-------------------------------------------------------------------

class AffectorColorRampHeight;
class CDib;
class Image;

#include "FormLayerItem.h"

//-------------------------------------------------------------------

class FormAffectorColorRampHeight : public FormLayerItem
{
private:

	AffectorColorRampHeight* affector;

	CDib* m_dib;
	int   m_dibSize;

private:

	Image* verifyImage (const CString& name, bool verbose=true) const;
	void   createDib (const Image* image);

private:

	//{{AFX_DATA(FormAffectorColorRampHeight)
	enum { IDD = IDD_FORM_AFFECTORCOLORRAMPHEIGHT };
	NumberEdit	m_highHeight;
	NumberEdit	m_lowHeight;
	CStatic	m_rampWindow;
	CString	m_imageName;
	CString	m_name;
	int		m_operation;
	//}}AFX_DATA

protected:

	FormAffectorColorRampHeight();
	DECLARE_DYNCREATE(FormAffectorColorRampHeight)
	virtual ~FormAffectorColorRampHeight();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(FormAffectorColorRampHeight)
	afx_msg void OnButtonBrowse();
	afx_msg void OnButtonRefresh();
	afx_msg void OnEditchangeOperation();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormAffectorColorRampHeight)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
