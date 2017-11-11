//
// FormAffectorHeightTerrace.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormAffectorHeightTerrace.h"

#include "sharedTerrain/AffectorHeight.h"
#include "CDib.h"
#include "FractalPreviewFrame.h"
#include "MainFrame.h"
#include "TerrainEditor.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormAffectorHeightTerrace, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

//-------------------------------------------------------------------

FormAffectorHeightTerrace::FormAffectorHeightTerrace() : 
	FormLayerItem(FormAffectorHeightTerrace::IDD),
	m_affector (0),

	//-- widgets
	m_editFraction (this),
	m_height (false),
	m_name (),
	m_sliderFraction ()
{
	//{{AFX_DATA_INIT(FormAffectorHeightTerrace)
	m_name = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------

FormAffectorHeightTerrace::~FormAffectorHeightTerrace()
{
	m_affector = 0;
}

//-------------------------------------------------------------------

void FormAffectorHeightTerrace::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormAffectorHeightTerrace)
	DDX_Control(pDX, IDC_HEIGHT, m_height);
	DDX_Control(pDX, IDC_FLATRATIO, m_editFraction);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Control(pDX, IDC_SLIDER_FLATRATIO, m_sliderFraction);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormAffectorHeightTerrace, PropertyView)
	//{{AFX_MSG_MAP(FormAffectorHeightTerrace)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormAffectorHeightTerrace::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormAffectorHeightTerrace::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormAffectorHeightTerrace::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	m_affector = dynamic_cast<AffectorHeightTerrace*> (flivd->item->layerItem);
	NOT_NULL (m_affector);
}

//-------------------------------------------------------------------

void FormAffectorHeightTerrace::OnInitialUpdate() 
{
	PropertyView::OnInitialUpdate();

	NOT_NULL (m_affector);

	m_name = m_affector->getName ();
	m_height = m_affector->getHeight ();
	
	m_editFraction.LinkSmartSliderCtrl (&m_sliderFraction);
	m_editFraction.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editFraction.SetValue (m_affector->getFraction ()));

	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormAffectorHeightTerrace::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormAffectorHeightTerrace::HasChanged () const
{
	return true;
}

//-------------------------------------------------------------------

void FormAffectorHeightTerrace::ApplyChanges ()
{
	if (!m_initialized)
		return;

	//-- update the controls/data
	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (m_affector);

		//-- update affector parameters
		CString data;
		m_editFraction.GetWindowText (data);
		m_affector->setFraction (static_cast<real> (atof (data)));
		m_affector->setHeight (m_height);

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

