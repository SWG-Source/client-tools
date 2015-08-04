//
// FormBitmapFamily.h
//
// copyright 2004, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_FormBitmapFamily_H
#define INCLUDED_FormBitmapFamily_H

//-------------------------------------------------------------------

class BitmapGroup;
class Image;

#include "PropertyView.h"
#include "SmartEditCtrl.h"
#include "SmartSliderCtrl.h"

//-------------------------------------------------------------------

class FormBitmapFamily : public PropertyView
{
public:

	class FormBitmapFamilyViewData : public PropertyView::ViewData
	{
	public:

		int           familyId;     //lint !e1925  //-- public data member
		BitmapGroup* bitmapGroup;  //lint !e1925  //-- public data member

	public:

		FormBitmapFamilyViewData ();
		virtual ~FormBitmapFamilyViewData ();
	};

private:

	FormBitmapFamilyViewData data;

	Image*  m_image;
	CToolTipCtrl   m_tooltip;

private:

	//{{AFX_DATA(FormBitmapFamily)
	enum { IDD = IDD_FORM_BITMAPFAMILY };
	CString	m_name;
	//}}AFX_DATA

private:

	void updateBitmap () const;

protected:

	FormBitmapFamily();           
	DECLARE_DYNCREATE(FormBitmapFamily)
	virtual ~FormBitmapFamily();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(FormBitmapFamily)
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	virtual void Initialize (PropertyView::ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(FormBitmapFamily)
	public:
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
