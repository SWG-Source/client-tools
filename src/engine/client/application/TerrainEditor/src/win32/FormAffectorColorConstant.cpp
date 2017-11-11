//
// FormAffectorColorConstant.cpp
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormAffectorColorConstant.h"

#include "sharedTerrain/AffectorColor.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormAffectorColorConstant, FormLayerItem)

//-------------------------------------------------------------------

FormAffectorColorConstant::FormAffectorColorConstant() : 
	FormLayerItem(FormAffectorColorConstant::IDD),
	m_affector (0),
	m_color (),

	//-- widgets
	m_windowColor (),
	m_name (),
	m_operation (0)
{
	//{{AFX_DATA_INIT(FormAffectorColorConstant)
	m_name = _T("");
	m_operation = -1;
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------
	
FormAffectorColorConstant::~FormAffectorColorConstant()
{
	m_affector = 0;
}

//-------------------------------------------------------------------

void FormAffectorColorConstant::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormAffectorColorConstant)
	DDX_Control(pDX, IDC_STATIC_WINDOW_COLOR, m_windowColor);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_CBIndex(pDX, IDC_OPERATION, m_operation);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FormAffectorColorConstant, PropertyView)
	//{{AFX_MSG_MAP(FormAffectorColorConstant)
	ON_BN_CLICKED(IDC_CHOOSECOLOR, OnChoosecolor)
	ON_CBN_EDITCHANGE(IDC_OPERATION, OnEditchangeOperation)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormAffectorColorConstant::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormAffectorColorConstant::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormAffectorColorConstant::OnDraw(CDC* pDC) 
{
	PropertyView::OnDraw (pDC);

	CRect rect;
	m_windowColor.GetWindowRect (&rect);
	ScreenToClient (&rect);
	
	CBrush brush;
	IGNORE_RETURN (brush.CreateSolidBrush (RGB (m_color.r, m_color.g, m_color.b)));
	IGNORE_RETURN (pDC->SelectObject (&brush));
	IGNORE_RETURN (pDC->Rectangle (&rect));
}

//-------------------------------------------------------------------

void FormAffectorColorConstant::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	m_affector = dynamic_cast<AffectorColorConstant*> (flivd->item->layerItem);
	NOT_NULL (m_affector);
}

//-------------------------------------------------------------------

void FormAffectorColorConstant::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (m_affector);

	m_name = m_affector->getName ();
	m_operation = static_cast<int> (m_affector->getOperation ());
	m_color = m_affector->getColor ();

	IGNORE_RETURN (UpdateData(false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormAffectorColorConstant::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormAffectorColorConstant::HasChanged (void) const
{
	return true;
}

//-------------------------------------------------------------------

void FormAffectorColorConstant::ApplyChanges (void)
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (m_affector);

		m_affector->setOperation (static_cast<TerrainGeneratorOperation> (m_operation));	
		m_affector->setColor (m_color);

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormAffectorColorConstant::OnChoosecolor() 
{
	NOT_NULL (m_affector);

	CColorDialog dlg (RGB (m_color.r, m_color.g, m_color.b));  //lint !e1924  //-- c-style cast
	if (dlg.DoModal () == IDOK)
	{
		m_color.r = GetRValue (dlg.m_cc.rgbResult);  //lint !e1924  //-- c-style cast
		m_color.g = GetGValue (dlg.m_cc.rgbResult);  //lint !e1924  //-- c-style cast
		m_color.b = GetBValue (dlg.m_cc.rgbResult);  //lint !e1924  //-- c-style cast

		Invalidate ();
	}
}

//-------------------------------------------------------------------

void FormAffectorColorConstant::OnEditchangeOperation() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

