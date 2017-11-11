//
// FormFilterBitmap.cpp
//
// copyright 2004, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormFilterBitmap.h"

#include "CDib.h"
#include "sharedTerrain/Filter.h"
#include "BitmapPreviewFrame.h"
#include "MainFrame.h"
#include "TerrainEditor.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormFilterBitmap, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormFilterBitmap::FormFilterBitmap() : 
	FormLayerItem(FormFilterBitmap::IDD),
	m_filter (0),
	m_tooltip (),

	//-- widgets
	m_familyCtrl (),
	m_editLow (this),
	m_editHigh (this),
	m_editFeatherDistance (this),
	m_sliderLow (),
	m_sliderHigh (),
	m_sliderGain (),
	m_editGain (this),
	m_sliderFeatherDistance (),
	m_featherFunction (),
	m_name (),
	m_operation (0)
{
	//{{AFX_DATA_INIT(FormFilterBitmap)
	m_name = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------

FormFilterBitmap::~FormFilterBitmap()
{
	m_filter = 0;
}

//-------------------------------------------------------------------

void FormFilterBitmap::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormFilterBitmap)
	DDX_Control(pDX, IDC_FAMILY, m_familyCtrl);
	DDX_Control(pDX, IDC_LOW, m_editLow);
	DDX_Control(pDX, IDC_HIGH, m_editHigh);
	DDX_Control(pDX, IDC_FEATHER_DISTANCE, m_editFeatherDistance);
	DDX_Control(pDX, IDC_FEATHER_FUNCTION, m_featherFunction);
	DDX_Control(pDX, IDC_SLIDER_LOW, m_sliderLow);
	DDX_Control(pDX, IDC_SLIDER_HIGH, m_sliderHigh);
	DDX_Control(pDX, IDC_SLIDER_BITMAP_GAIN, m_sliderGain);
	DDX_Control(pDX, IDC_BITMAP_GAIN, m_editGain);
	DDX_Control(pDX, IDC_SLIDER_FEATHERDISTANCE, m_sliderFeatherDistance);
	DDX_Text(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormFilterBitmap, PropertyView)
	//{{AFX_MSG_MAP(FormFilterBitmap)
	ON_CBN_SELCHANGE(IDC_FEATHER_FUNCTION, OnSelchangeFeatherFunction)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_FAMILY, OnSelchangeFamily)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormFilterBitmap::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormFilterBitmap::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormFilterBitmap::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* bld = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (bld);

	m_filter = dynamic_cast<FilterBitmap*> (bld->item->layerItem);
	NOT_NULL (m_filter);
}

//-------------------------------------------------------------------

void FormFilterBitmap::OnInitialUpdate() 
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
	doc->BuildBitmapFamilyDropList (m_familyCtrl, m_filter->getFamilyId ());

	IGNORE_RETURN (m_featherFunction.SetCurSel (m_filter->getFeatherFunction ()));

	m_editFeatherDistance.LinkSmartSliderCtrl (&m_sliderFeatherDistance);
	m_editFeatherDistance.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editFeatherDistance.SetValue (m_filter->getFeatherDistance ()));
		
	m_editLow.LinkSmartSliderCtrl (&m_sliderLow);
	m_editLow.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editLow.SetValue (m_filter->getLowBitmapLimit ()));

	m_editHigh.LinkSmartSliderCtrl (&m_sliderHigh);
	m_editHigh.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editHigh.SetValue (m_filter->getHighBitmapLimit ()));

	m_editGain.LinkSmartSliderCtrl (&m_sliderGain);
	m_editGain.SetParams(-1.0f,1.0f,20,"%1.5f");
	IGNORE_RETURN (m_editGain.SetValue (m_filter->getGain ()));

	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;

	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormFilterBitmap::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormFilterBitmap::HasChanged () const
{
	return true;
}

//-------------------------------------------------------------------

void FormFilterBitmap::ApplyChanges ()
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

		m_editGain.GetWindowText (data);
		m_filter->setGain(static_cast<real> (atof (data)));

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


		m_filter->setLowBitmapLimit (low);
		m_filter->setHighBitmapLimit (high);

		safe_cast<TerrainEditorApp*> (AfxGetApp ())->showBitmapPreview ();
		GetParentFrame ()->ActivateFrame ();

		TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
		NOT_NULL (doc);
		
		const Image* image = doc->getTerrainGenerator ()->getBitmapGroup ().getFamilyBitmap (m_filter->getFamilyId ());
		NOT_NULL (image);

		if (doc->getBitmapPreviewFrame ())
		{
			doc->getBitmapPreviewFrame ()->updateBitmap (*image, low, high, m_filter->getGain());
		}

		//-- tell the document we've modified data
		GetDocument ()->SetModifiedFlag (true);
		GetDocument ()->UpdateAllViews (this);
	}
}

//-------------------------------------------------------------------

void FormFilterBitmap::OnSelchangeFeatherFunction() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

BOOL FormFilterBitmap::PreTranslateMessage(MSG* pMsg) 
{
	m_tooltip.RelayEvent(pMsg);
	
	return PropertyView::PreTranslateMessage(pMsg);
}

//-------------------------------------------------------------------

void FormFilterBitmap::OnSelchangeFamily() 
{
	int famId = m_familyCtrl.GetItemData (m_familyCtrl.GetCurSel ());
	TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	NOT_NULL (doc);
		
	const Image* image = doc->getTerrainGenerator ()->getBitmapGroup ().getFamilyBitmap (famId);

	if(!image)
	{
		
		MessageBox("image not set for selected family yet");
		int numItems = m_familyCtrl.GetCount();
		for(int i = 0; i < numItems;++i)
		{
			int famId = m_familyCtrl.GetItemData (i);
			const Image* image = doc->getTerrainGenerator ()->getBitmapGroup ().getFamilyBitmap (famId);
			if(image)
			{
				m_familyCtrl.SetCurSel(i);
				break;
			}

		}
		
	}
	ApplyChanges();
}

//-------------------------------------------------------------------

