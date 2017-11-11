//
// FormFilterFractal.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormFilterFractal.h"

#include "CDib.h"
#include "sharedTerrain/Filter.h"
#include "FractalPreviewFrame.h"
#include "MainFrame.h"
#include "TerrainEditor.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormFilterFractal, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormFilterFractal::FormFilterFractal() : 
	FormLayerItem(FormFilterFractal::IDD),
	m_filter (0),
	m_tooltip (),

	//-- widgets
	m_familyCtrl (),
	m_editLow (this),
	m_editHigh (this),
	m_editFeatherDistance (this),
	m_sliderLow (),
	m_sliderHigh (),
	m_sliderFeatherDistance (),
	m_featherFunction (),
	m_name (),
	m_operation (0)
{
	//{{AFX_DATA_INIT(FormFilterFractal)
	m_name = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------

FormFilterFractal::~FormFilterFractal()
{
	m_filter = 0;
}

//-------------------------------------------------------------------

void FormFilterFractal::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormFilterFractal)
	DDX_Control(pDX, IDC_FAMILY, m_familyCtrl);
	DDX_Control(pDX, IDC_LOW, m_editLow);
	DDX_Control(pDX, IDC_HIGH, m_editHigh);
	DDX_Control(pDX, IDC_FEATHER_DISTANCE, m_editFeatherDistance);
	DDX_Control(pDX, IDC_FEATHER_FUNCTION, m_featherFunction);
	DDX_Control(pDX, IDC_SLIDER_LOW, m_sliderLow);
	DDX_Control(pDX, IDC_SLIDER_HIGH, m_sliderHigh);
	DDX_Control(pDX, IDC_SLIDER_FEATHERDISTANCE, m_sliderFeatherDistance);
	DDX_Text(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormFilterFractal, PropertyView)
	//{{AFX_MSG_MAP(FormFilterFractal)
	ON_CBN_SELCHANGE(IDC_FEATHER_FUNCTION, OnSelchangeFeatherFunction)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_FAMILY, OnSelchangeFamily)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormFilterFractal::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormFilterFractal::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormFilterFractal::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* bld = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (bld);

	m_filter = dynamic_cast<FilterFractal*> (bld->item->layerItem);
	NOT_NULL (m_filter);
}

//-------------------------------------------------------------------

void FormFilterFractal::OnInitialUpdate() 
{
	PropertyView::OnInitialUpdate();

	{
		IGNORE_RETURN (m_tooltip.Create(this));
		m_tooltip.Activate(TRUE);
		IGNORE_RETURN (m_tooltip.SetMaxTipWidth (150));

		IGNORE_RETURN (m_tooltip.AddTool(GetDlgItem(IDC_LOW), IDS_LOW));
		IGNORE_RETURN (m_tooltip.AddTool(GetDlgItem(IDC_HIGH), IDS_HIGH));
		IGNORE_RETURN (m_tooltip.AddTool(GetDlgItem(IDC_FEATHER_DISTANCE), IDS_FEATHERDISTANCEFLOAT));
		IGNORE_RETURN (m_tooltip.AddTool(GetDlgItem(IDC_FEATHER_FUNCTION), IDS_FEATHERFUNCTION));
		// m_tooltip.AddTool(GetDlgItem(IDC_<name>), "<text>");
	}

	NOT_NULL (m_filter);

	m_name = m_filter->getName ();

	//-- update settings
	TerrainEditorDoc* const doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	doc->BuildFractalFamilyDropList (m_familyCtrl, m_filter->getFamilyId ());

	IGNORE_RETURN (m_featherFunction.SetCurSel (m_filter->getFeatherFunction ()));

	m_editFeatherDistance.LinkSmartSliderCtrl (&m_sliderFeatherDistance);
	m_editFeatherDistance.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editFeatherDistance.SetValue (m_filter->getFeatherDistance ()));
		
	m_editLow.LinkSmartSliderCtrl (&m_sliderLow);
	m_editLow.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editLow.SetValue (m_filter->getLowFractalLimit ()));

	m_editHigh.LinkSmartSliderCtrl (&m_sliderHigh);
	m_editHigh.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editHigh.SetValue (m_filter->getHighFractalLimit ()));

	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;

	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormFilterFractal::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormFilterFractal::HasChanged () const
{
	return true;
}

//-------------------------------------------------------------------

void FormFilterFractal::ApplyChanges ()
{
	if (!m_initialized)
		return;

	//-- update the controls/data
	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (m_filter);

		CString data;

		m_filter->setFamilyId (static_cast<int> (m_familyCtrl.GetItemData (m_familyCtrl.GetCurSel ())));

		//-- update the feathering
		m_filter->setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (m_featherFunction.GetCurSel ()));

		m_editFeatherDistance.GetWindowText (data);
		m_filter->setFeatherDistance (static_cast<real> (atof (data)));

		//-- update low/high
		m_editLow.GetWindowText (data);
		real low = static_cast<real> (atof (data));

		m_editHigh.GetWindowText (data);
		real high = static_cast<real> (atof (data));

		bool reset = false;
		if (low > high)
		{
			high = low;
			reset = true;
		}
		else
			if (high < low)
			{
				low = high;
				reset = true;
			}

		if (reset)
		{
			IGNORE_RETURN (m_editLow.SetValue (low));
			IGNORE_RETURN (m_editHigh.SetValue (high));
		}

		m_filter->setLowFractalLimit (low);
		m_filter->setHighFractalLimit (high);

		safe_cast<TerrainEditorApp*> (AfxGetApp ())->showFractalPreview ();
		GetParentFrame ()->ActivateFrame ();

		TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
		NOT_NULL (doc);
		
		const MultiFractal* multiFractal = doc->getTerrainGenerator ()->getFractalGroup ().getFamilyMultiFractal (m_filter->getFamilyId ());
		NOT_NULL (multiFractal);

		if (doc->getFractalPreviewFrame ())
			doc->getFractalPreviewFrame ()->updateBitmap (*multiFractal, low, high);

		//-- tell the document we've modified data
		GetDocument ()->SetModifiedFlag (true);
		GetDocument ()->UpdateAllViews (this);
	}
}

//-------------------------------------------------------------------

void FormFilterFractal::OnSelchangeFeatherFunction() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

BOOL FormFilterFractal::PreTranslateMessage(MSG* pMsg) 
{
	m_tooltip.RelayEvent(pMsg);
	
	return PropertyView::PreTranslateMessage(pMsg);
}

//-------------------------------------------------------------------

void FormFilterFractal::OnSelchangeFamily() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

