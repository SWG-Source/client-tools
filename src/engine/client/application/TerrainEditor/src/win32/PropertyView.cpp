//
// PropertyView.cpp
// asommers 
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------
 
#include "FirstTerrainEditor.h"
#include "PropertyView.h"

//-------------------------------------------------------------------

PropertyView::ViewData::ViewData (void)
{
}

//-------------------------------------------------------------------

PropertyView::ViewData::~ViewData (void)
{
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(PropertyView, CFormView)

//-------------------------------------------------------------------

PropertyView::PropertyView() : 
	CFormView(PropertyView::IDD),
	m_initialized (false)
{
	//{{AFX_DATA_INIT(PropertyView)
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------
	
PropertyView::PropertyView (UINT nIDTemplate) : 
	CFormView(nIDTemplate),
	m_initialized (false)
{
}

//-------------------------------------------------------------------
	
PropertyView::~PropertyView()
{
}

//-------------------------------------------------------------------

void PropertyView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PropertyView)
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(PropertyView, CFormView)
	//{{AFX_MSG_MAP(PropertyView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

#ifdef _DEBUG
void PropertyView::AssertValid() const
{
	CFormView::AssertValid();
}

void PropertyView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void PropertyView::Initialize (ViewData* vd)
{
	UNREF (vd);
}

//-------------------------------------------------------------------

bool PropertyView::HasChanged () const
{
	return false;
}

//-------------------------------------------------------------------

void PropertyView::ApplyChanges ()
{
}

//-------------------------------------------------------------------

void PropertyView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	m_initialized = false;
}

//-------------------------------------------------------------------

