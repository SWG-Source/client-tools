//
// FormFilterHeight.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormFilterHeight.h"

#include "sharedTerrain/Filter.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormFilterHeight, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormFilterHeight::FormFilterHeight() : 
	FormLayerItem(FormFilterHeight::IDD),
	m_filter (0),
	m_editFeatherDistance (this),
	m_sliderFeatherDistance (),

	//-- widgets
	m_featherFunction (),
	m_highHeight (true),
	m_lowHeight (true),
	m_name ()
{
	//{{AFX_DATA_INIT(FormFilterHeight)
	m_name = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------

FormFilterHeight::~FormFilterHeight()
{
	m_filter = 0;
}

//-------------------------------------------------------------------

void FormFilterHeight::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormFilterHeight)
	DDX_Control(pDX, IDC_LOWHT, m_lowHeight);
	DDX_Control(pDX, IDC_HIGHHT, m_highHeight);
	DDX_Control(pDX, IDC_SLIDER_FEATHERDISTANCE, m_sliderFeatherDistance);
	DDX_Control(pDX, IDC_FEATHER_DISTANCE, m_editFeatherDistance);
	DDX_Control(pDX, IDC_FEATHER_FUNCTION, m_featherFunction);
	DDX_Text(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormFilterHeight, PropertyView)
	//{{AFX_MSG_MAP(FormFilterHeight)
	ON_CBN_SELCHANGE(IDC_FEATHER_FUNCTION, OnSelchangeFeatherFunction)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormFilterHeight::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormFilterHeight::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormFilterHeight::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	m_filter = dynamic_cast<FilterHeight*> (flivd->item->layerItem);
	NOT_NULL (m_filter);
}

//-------------------------------------------------------------------

void FormFilterHeight::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (m_filter);

	m_name       = m_filter->getName();
	m_lowHeight  = m_filter->getLowHeight ();
	m_highHeight = m_filter->getHighHeight ();
	IGNORE_RETURN (m_featherFunction.SetCurSel (m_filter->getFeatherFunction ()));
	m_editFeatherDistance.LinkSmartSliderCtrl (&m_sliderFeatherDistance);
	m_editFeatherDistance.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editFeatherDistance.SetValue (m_filter->getFeatherDistance ()));

	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormFilterHeight::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormFilterHeight::HasChanged () const
{
	return 
		(m_lowHeight  != m_filter->getLowHeight ()) ||
		(m_highHeight != m_filter->getHighHeight ()) ||
		(static_cast<TerrainGeneratorFeatherFunction> (m_featherFunction.GetCurSel ()) != m_filter->getFeatherFunction ()) ||
		(m_editFeatherDistance.GetValueFloat () != m_filter->getFeatherDistance ());
}

//-------------------------------------------------------------------

void FormFilterHeight::ApplyChanges ()
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (m_filter);

		m_filter->setLowHeight (m_lowHeight);
		m_filter->setHighHeight (m_highHeight);
		m_filter->setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (m_featherFunction.GetCurSel ()));
		m_filter->setFeatherDistance (m_editFeatherDistance.GetValueFloat ());

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormFilterHeight::OnSelchangeFeatherFunction() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------
