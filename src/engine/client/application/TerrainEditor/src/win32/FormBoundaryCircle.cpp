//
// FormBoundaryCircle.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormBoundaryCircle.h"

#include "sharedTerrain/Boundary.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormBoundaryCircle, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormBoundaryCircle::FormBoundaryCircle() : 
	FormLayerItem(FormBoundaryCircle::IDD),
	boundary (0),
	m_editFeatherDistance (this),
	m_sliderFeatherDistance (),

	//-- widgets
	m_featherFunction (),
	m_centerX (true),
	m_centerZ (true),
	m_name (),
	m_radius (true)
{
	//{{AFX_DATA_INIT(FormBoundaryCircle)
	m_name = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------
	
FormBoundaryCircle::~FormBoundaryCircle()
{
	boundary = 0;
}

//-------------------------------------------------------------------

void FormBoundaryCircle::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormBoundaryCircle)
	DDX_Control(pDX, IDC_RX, m_radius);
	DDX_Control(pDX, IDC_CZ, m_centerZ);
	DDX_Control(pDX, IDC_CX, m_centerX);
	DDX_Control(pDX, IDC_SLIDER_FEATHERDISTANCE, m_sliderFeatherDistance);
	DDX_Control(pDX, IDC_FEATHER_DISTANCE, m_editFeatherDistance);
	DDX_Control(pDX, IDC_FEATHER_FUNCTION, m_featherFunction);
	DDX_Text(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormBoundaryCircle, PropertyView)
	//{{AFX_MSG_MAP(FormBoundaryCircle)
	ON_CBN_SELCHANGE(IDC_FEATHER_FUNCTION, OnSelchangeFeatherFunction)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormBoundaryCircle::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormBoundaryCircle::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormBoundaryCircle::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* const flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	boundary = dynamic_cast<BoundaryCircle*> (flivd->item->layerItem);
	NOT_NULL (boundary);
}

//-------------------------------------------------------------------

void FormBoundaryCircle::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	update ();

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormBoundaryCircle::update (void) 
{
	NOT_NULL (boundary);

	m_name     = boundary->getName();
	m_centerX  = boundary->getCenterX ();
	m_centerZ  = boundary->getCenterZ ();
	m_radius   = boundary->getRadius ();
	IGNORE_RETURN (m_featherFunction.SetCurSel (boundary->getFeatherFunction ()));
	m_editFeatherDistance.LinkSmartSliderCtrl (&m_sliderFeatherDistance);
	m_editFeatherDistance.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editFeatherDistance.SetValue (boundary->getFeatherDistance ()));

	IGNORE_RETURN (UpdateData (false));
}

//-------------------------------------------------------------------

void FormBoundaryCircle::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormBoundaryCircle::HasChanged () const
{
	return 
		(m_centerX         != boundary->getCenterX ()) ||
		(m_centerZ         != boundary->getCenterZ ()) ||
		(m_radius          != boundary->getRadius ()) ||
		(m_featherFunction.GetCurSel () != boundary->getFeatherFunction ()) ||
		(m_editFeatherDistance.GetValueFloat () != boundary->getFeatherDistance ());
}

//-------------------------------------------------------------------

void FormBoundaryCircle::ApplyChanges ()
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (boundary);

		m_radius = fabsf (m_radius);

		IGNORE_RETURN (UpdateData (false));

		boundary->setCircle (m_centerX, m_centerZ, m_radius);
		boundary->setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (m_featherFunction.GetCurSel ()));
		boundary->setFeatherDistance (m_editFeatherDistance.GetValueFloat ());

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormBoundaryCircle::OnSelchangeFeatherFunction() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormBoundaryCircle::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	UNREF (pSender);
	UNREF (lHint);
	UNREF (pHint);

	if (pSender)
	{
		update ();
	}
}

//-------------------------------------------------------------------
