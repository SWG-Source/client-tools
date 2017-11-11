//
// FormAffectorPassable.cpp
// asommers
// 
// copyright 2000, sony online entertainment

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormAffectorPassable.h"

#include "sharedTerrain/AffectorPassable.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormAffectorPassable, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormAffectorPassable::FormAffectorPassable() : 
	FormLayerItem(FormAffectorPassable::IDD),
	affector (0),

	//-- widgets
	m_editFeatherClamp (this),
	m_sliderFeatherClamp (),
	m_passable (FALSE)
{
	//{{AFX_DATA_INIT(FormAffectorPassable)
	m_passable = FALSE;
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------
	
FormAffectorPassable::~FormAffectorPassable()
{
	affector = 0;
}

//-------------------------------------------------------------------

void FormAffectorPassable::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormAffectorPassable)
	DDX_Control(pDX, IDC_SLIDER_FEATHERCLAMP, m_sliderFeatherClamp);
	DDX_Control(pDX, IDC_EDIT_FEATHERCLAMP, m_editFeatherClamp);
	DDX_Check(pDX, IDC_CHECK_PASSABLE, m_passable);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormAffectorPassable, PropertyView)
	//{{AFX_MSG_MAP(FormAffectorPassable)
	ON_CBN_SELCHANGE(IDC_FAMILY, OnSelchangefamily)
	ON_BN_CLICKED(IDC_CHECK_PASSABLE, OnCheckPassable)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormAffectorPassable::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormAffectorPassable::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormAffectorPassable::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	affector = dynamic_cast<AffectorPassable*>(flivd->item->layerItem);
	NOT_NULL (affector);
}

//-------------------------------------------------------------------

void FormAffectorPassable::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (affector);

	m_passable   = affector->isPassable() ? TRUE : FALSE;
	m_editFeatherClamp.LinkSmartSliderCtrl (&m_sliderFeatherClamp);
	m_editFeatherClamp.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editFeatherClamp.SetValue (affector->getFeatherThreshold()));

	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormAffectorPassable::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormAffectorPassable::HasChanged () const
{
	return 
		((m_passable == TRUE)    != affector->isPassable()) ||
		(m_editFeatherClamp.GetValueFloat () != affector->getFeatherThreshold ());
}

//-------------------------------------------------------------------

void FormAffectorPassable::ApplyChanges (void)
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (affector);

		affector->setPassable(m_passable == TRUE);
		affector->setFeatherThreshold(m_editFeatherClamp.GetValueFloat ());

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormAffectorPassable::OnSelchangefamily() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorPassable::OnCheckPassable() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

