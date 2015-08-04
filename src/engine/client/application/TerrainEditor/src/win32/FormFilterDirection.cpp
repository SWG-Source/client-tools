//
// FormFilterDirection.cpp 
// asommers
//
// copyright 2001, sony online entertainment

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormFilterDirection.h"

#include "sharedTerrain/Filter.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormFilterDirection, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormFilterDirection::FormFilterDirection() : 
	FormLayerItem(FormFilterDirection::IDD),
	m_filter (0),
	m_editFeatherDistance (this),
	m_sliderFeatherDistance (),

	//-- widgets
	m_featherFunction (),
	m_maximumAngleDegrees (true),
	m_minimumAngleDegrees (true),
	m_name ()
{
	//{{AFX_DATA_INIT(FormFilterDirection)
	m_name = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------

FormFilterDirection::~FormFilterDirection()
{
	m_filter = 0;
}

//-------------------------------------------------------------------

void FormFilterDirection::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormFilterDirection)
	DDX_Control(pDX, IDC_MINANGDEG, m_minimumAngleDegrees);
	DDX_Control(pDX, IDC_MAXANGLEDEG, m_maximumAngleDegrees);
	DDX_Control(pDX, IDC_SLIDER_FEATHERDISTANCE, m_sliderFeatherDistance);
	DDX_Control(pDX, IDC_FEATHER_DISTANCE, m_editFeatherDistance);
	DDX_Control(pDX, IDC_FEATHER_FUNCTION, m_featherFunction);
	DDX_Text(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormFilterDirection, PropertyView)
	//{{AFX_MSG_MAP(FormFilterDirection)
	ON_CBN_SELCHANGE(IDC_FEATHER_FUNCTION, OnSelchangeFeatherFunction)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormFilterDirection::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormFilterDirection::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormFilterDirection::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	m_filter = dynamic_cast<FilterDirection*> (flivd->item->layerItem);
	NOT_NULL (m_filter);
}

//-------------------------------------------------------------------

void FormFilterDirection::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (m_filter);

	m_name    = m_filter->getName();
	m_minimumAngleDegrees = convertRadiansToDegrees (m_filter->getMinimumAngle ());
	m_maximumAngleDegrees = convertRadiansToDegrees (m_filter->getMaximumAngle ());
	IGNORE_RETURN (m_featherFunction.SetCurSel (m_filter->getFeatherFunction ()));
	m_editFeatherDistance.LinkSmartSliderCtrl (&m_sliderFeatherDistance);
	m_editFeatherDistance.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editFeatherDistance.SetValue (m_filter->getFeatherDistance ()));

	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormFilterDirection::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormFilterDirection::HasChanged () const
{
	return
		(m_minimumAngleDegrees != convertRadiansToDegrees (m_filter->getMinimumAngle ())) ||
		(m_maximumAngleDegrees != convertRadiansToDegrees (m_filter->getMaximumAngle ())) ||
		(static_cast<TerrainGeneratorFeatherFunction> (m_featherFunction.GetCurSel ()) != m_filter->getFeatherFunction ()) ||
		(m_editFeatherDistance.GetValueFloat () != m_filter->getFeatherDistance ());
}

//-------------------------------------------------------------------

void FormFilterDirection::ApplyChanges ()
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (m_filter);

		m_filter->setMinimumAngle (convertDegreesToRadians (m_minimumAngleDegrees));
		m_filter->setMaximumAngle (convertDegreesToRadians (m_maximumAngleDegrees));
		m_filter->setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (m_featherFunction.GetCurSel ()));
		m_filter->setFeatherDistance (m_editFeatherDistance.GetValueFloat ());

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormFilterDirection::OnSelchangeFeatherFunction() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

