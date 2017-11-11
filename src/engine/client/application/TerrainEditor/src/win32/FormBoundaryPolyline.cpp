//
// FormBoundaryPolyline.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormBoundaryPolyline.h"

#include "DialogControlPointEditor.h"
#include "sharedTerrain/Boundary.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormBoundaryPolyline, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormBoundaryPolyline::FormBoundaryPolyline() : 
	FormLayerItem(FormBoundaryPolyline::IDD),
	boundary (0),
	m_editFeatherDistance (this),
	m_sliderFeatherDistance (),

	//-- widgets
	m_featherFunction (),
	m_width (true),
	m_name ()
{
	//{{AFX_DATA_INIT(FormBoundaryPolyline)
	m_name = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------

FormBoundaryPolyline::~FormBoundaryPolyline()
{
	boundary = 0;
}

//-------------------------------------------------------------------

void FormBoundaryPolyline::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormBoundaryPolyline)
	DDX_Control(pDX, IDC_WIDTH, m_width);
	DDX_Control(pDX, IDC_SLIDER_FEATHERDISTANCE, m_sliderFeatherDistance);
	DDX_Control(pDX, IDC_FEATHER_DISTANCE, m_editFeatherDistance);
	DDX_Control(pDX, IDC_FEATHER_FUNCTION, m_featherFunction);
	DDX_Text(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormBoundaryPolyline, PropertyView)
	//{{AFX_MSG_MAP(FormBoundaryPolyline)
	ON_CBN_SELCHANGE(IDC_FEATHER_FUNCTION, OnSelchangeFeatherFunction)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CONTROL_POINT_LIST, OnButtonEditControlPointList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormBoundaryPolyline::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormBoundaryPolyline::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormBoundaryPolyline::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (boundary);

	m_name     = boundary->getName();
	m_width    = boundary->getWidth ();
	IGNORE_RETURN (m_featherFunction.SetCurSel (boundary->getFeatherFunction ()));
	m_editFeatherDistance.LinkSmartSliderCtrl (&m_sliderFeatherDistance);
	m_editFeatherDistance.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editFeatherDistance.SetValue (boundary->getFeatherDistance ()));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormBoundaryPolyline::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* const flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	boundary = dynamic_cast<BoundaryPolyline*> (flivd->item->layerItem);
	NOT_NULL (boundary);
}

//-------------------------------------------------------------------

void FormBoundaryPolyline::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormBoundaryPolyline::HasChanged () const
{
	return
		(m_width           != boundary->getWidth ()) ||
		(m_featherFunction.GetCurSel () != boundary->getFeatherFunction ()) ||
		(m_editFeatherDistance.GetValueFloat () != boundary->getFeatherDistance ());
}

//-------------------------------------------------------------------

void FormBoundaryPolyline::ApplyChanges ()
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (boundary);

		boundary->setWidth (m_width);
		boundary->setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (m_featherFunction.GetCurSel ()));
		boundary->setFeatherDistance (m_editFeatherDistance.GetValueFloat ());

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormBoundaryPolyline::OnSelchangeFeatherFunction() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormBoundaryPolyline::OnButtonEditControlPointList() 
{
	DialogControlPointEditor dlg (boundary->getPointList ());
	if (dlg.DoModal ())
	{
		boundary->copyPointList (dlg.getPointList ());
		ApplyChanges ();
	}
}

//-------------------------------------------------------------------

