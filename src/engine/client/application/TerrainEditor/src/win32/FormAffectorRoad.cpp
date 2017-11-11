//
// FormAffectorRoad.cpp
// asommers
// 
// copyright 2000, sony online entertainment

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormAffectorRoad.h"

#include "DialogControlPointEditor.h"
#include "sharedTerrain/AffectorRoad.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormAffectorRoad, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormAffectorRoad::FormAffectorRoad() : 
	FormLayerItem(FormAffectorRoad::IDD),
	affector (0),
	m_sliderFeatherDistanceShader (),
	m_editFeatherDistanceShader (this),
	m_sliderFeatherDistance (),
	m_editFeatherDistance (this),

	//-- widgets
	m_width (false),
	m_featherFunctionShader (),
	m_featherFunction (),
	m_familyCtl (),
	m_hasFixedHeights (FALSE),
	m_name ()
{
	//{{AFX_DATA_INIT(FormAffectorRoad)
	m_hasFixedHeights = FALSE;
	m_name = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------
	
FormAffectorRoad::~FormAffectorRoad()
{
	affector = 0;
}

//-------------------------------------------------------------------

void FormAffectorRoad::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormAffectorRoad)
	DDX_Control(pDX, IDC_SLIDER_FEATHERDISTANCESHADER, m_sliderFeatherDistanceShader);
	DDX_Control(pDX, IDC_FEATHER_FUNCTIONSHADER, m_featherFunctionShader);
	DDX_Control(pDX, IDC_FEATHER_DISTANCESHADER, m_editFeatherDistanceShader);
	DDX_Control(pDX, IDC_EDIT_WIDTH2, m_width);
	DDX_Control(pDX, IDC_SLIDER_FEATHERDISTANCE, m_sliderFeatherDistance);
	DDX_Control(pDX, IDC_FEATHER_DISTANCE, m_editFeatherDistance);
	DDX_Control(pDX, IDC_FEATHER_FUNCTION, m_featherFunction);
	DDX_Control(pDX, IDC_FAMILY, m_familyCtl);
	DDX_Check(pDX, IDC_CHECK_HAS_FIXED_HEIGHTS, m_hasFixedHeights);
	DDX_Text(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormAffectorRoad, PropertyView)
	//{{AFX_MSG_MAP(FormAffectorRoad)
	ON_CBN_SELCHANGE(IDC_FAMILY, OnSelchangeFamily)
	ON_CBN_SELCHANGE(IDC_FEATHER_FUNCTION, OnSelchangeFeatherFunction)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK_HAS_FIXED_HEIGHTS, OnCheckHasFixedHeights)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CONTROL_POINT_LIST, OnButtonEditControlPointList)
	ON_CBN_SELCHANGE(IDC_FEATHER_FUNCTIONSHADER, OnSelchangeFeatherFunctionshader)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormAffectorRoad::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormAffectorRoad::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormAffectorRoad::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	affector = dynamic_cast<AffectorRoad*>(flivd->item->layerItem);
	NOT_NULL (affector);
}

//-------------------------------------------------------------------

void FormAffectorRoad::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (affector);

	m_name   = affector->getName ();
	m_width  = affector->getWidth ();
	m_hasFixedHeights = affector->getHasFixedHeights();
	IGNORE_RETURN (m_featherFunction.SetCurSel (affector->getFeatherFunction ()));
	m_editFeatherDistance.LinkSmartSliderCtrl (&m_sliderFeatherDistance);
	m_editFeatherDistance.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editFeatherDistance.SetValue (affector->getFeatherDistance ()));

	IGNORE_RETURN (m_featherFunctionShader.SetCurSel (affector->getFeatherFunctionShader ()));
	m_editFeatherDistanceShader.LinkSmartSliderCtrl (&m_sliderFeatherDistanceShader);
	m_editFeatherDistanceShader.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editFeatherDistanceShader.SetValue (affector->getFeatherDistanceShader ()));

	TerrainEditorDoc* const doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	doc->BuildShaderFamilyDropList (m_familyCtl, affector->getFamilyId ());

	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormAffectorRoad::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormAffectorRoad::HasChanged () const
{
	return true;
}

//-------------------------------------------------------------------

void FormAffectorRoad::ApplyChanges ()
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (affector);

		affector->setWidth (m_width);
		affector->setFamilyId (static_cast<int> (m_familyCtl.GetItemData (m_familyCtl.GetCurSel ())));
		affector->setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (m_featherFunction.GetCurSel ()));
		affector->setFeatherDistance (m_editFeatherDistance.GetValueFloat ());
		affector->setFeatherFunctionShader (static_cast<TerrainGeneratorFeatherFunction> (m_featherFunctionShader.GetCurSel ()));
		affector->setFeatherDistanceShader (m_editFeatherDistanceShader.GetValueFloat ());
		affector->setHasFixedHeights (m_hasFixedHeights == TRUE);
		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormAffectorRoad::OnSelchangeFamily() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorRoad::OnSelchangeFeatherFunction() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorRoad::OnCheckHasFixedHeights() 
{
	ApplyChanges ();
	if(m_hasFixedHeights == TRUE)
	{
		affector->createInitialHeightList();
	}
	else
	{
		affector->clearHeightList();
	}
}

//-------------------------------------------------------------------

void FormAffectorRoad::OnButtonEditControlPointList() 
{
	if(m_hasFixedHeights == TRUE)
	{
		DialogControlPointEditor dlg (affector->getPointList(),affector->getHeightList());
		if (dlg.DoModal ())
		{
			affector->copyPointList (dlg.getPointList ());
			affector->copyHeightList (dlg.getHeightList ());
			ApplyChanges ();
		}
	}
	else
	{

		DialogControlPointEditor dlg (affector->getPointList ());
		if (dlg.DoModal ())
		{
			affector->copyPointList (dlg.getPointList ());
			ApplyChanges ();
		}
	}
}

//-------------------------------------------------------------------

void FormAffectorRoad::OnSelchangeFeatherFunctionshader() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

