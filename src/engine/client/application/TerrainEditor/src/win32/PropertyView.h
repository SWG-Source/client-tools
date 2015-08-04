//
// PropertyView.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_PropertyView_H
#define INCLUDED_PropertyView_H

//-------------------------------------------------------------------

#include "resource.h"

//-------------------------------------------------------------------

class PropertyView : public CFormView
{
public:

	class ViewData
	{
	public:

		ViewData ();
		virtual ~ViewData ();
	};

private:

	//{{AFX_DATA(PropertyView)
	enum { IDD = IDD_DEFAULT_PROPVIEW };
	//}}AFX_DATA

protected:

	PropertyView();           
	explicit PropertyView (UINT nIDTemplate);
	DECLARE_DYNCREATE(PropertyView)
	virtual ~PropertyView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(PropertyView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:

	bool m_initialized;

public:

	virtual void Initialize (ViewData* vd);
	virtual bool HasChanged () const;
	virtual void ApplyChanges ();

	//{{AFX_VIRTUAL(PropertyView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
};

//-------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}

//-------------------------------------------------------------------

#endif 
