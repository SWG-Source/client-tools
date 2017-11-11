//
// FormLayer.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormLayer.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormLayer, FormLayerItem)

//-------------------------------------------------------------------

FormLayer::FormLayer (void) : 
	FormLayerItem (FormLayer::IDD),
	layer (0),

	//-- widgets
	m_name (),
	m_invertBoundaries (FALSE),
	m_invertFilters (FALSE)
{
	//{{AFX_DATA_INIT(FormLayer)
	m_name = _T("");
	m_invertBoundaries = FALSE;
	m_invertFilters = FALSE;
	m_notes = _T("");
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------
	
FormLayer::~FormLayer()
{
	layer = 0;
}

//-------------------------------------------------------------------

void FormLayer::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(FormLayer)
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Check(pDX, IDC_CHECK_INVERT_BOUNDARIES, m_invertBoundaries);
	DDX_Check(pDX, IDC_CHECK_INVERT_FILTERS, m_invertFilters);
	DDX_Text(pDX, IDC_EDIT_NOTES, m_notes);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FormLayer, PropertyView)
	//{{AFX_MSG_MAP(FormLayer)
	ON_EN_CHANGE(IDC_EDIT_NOTES, OnChangeEditNotes)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormLayer::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormLayer::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormLayer::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	layer = dynamic_cast<TerrainGenerator::Layer*> (flivd->item->layerItem);
	NOT_NULL (layer);
}

//-------------------------------------------------------------------

void FormLayer::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (layer);

	m_name             = layer->getName();
	m_invertBoundaries = layer->getInvertBoundaries ();
	m_invertFilters    = layer->getInvertFilters ();
	m_notes            = layer->getNotes ();

	IGNORE_RETURN (UpdateData(false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormLayer::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormLayer::HasChanged () const
{
	return 
		((m_invertBoundaries == TRUE)   != layer->getInvertBoundaries ()) ||
		((m_invertFilters == TRUE)      != layer->getInvertFilters ()) ||
		(m_notes                        != (layer->getNotes () ? layer->getNotes () : ""));
}

//-------------------------------------------------------------------

void FormLayer::ApplyChanges (void)
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (layer);

		layer->setInvertBoundaries (m_invertBoundaries == TRUE);
		layer->setInvertFilters (m_invertFilters == TRUE);
		layer->setNotes (m_notes);

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormLayer::OnChangeEditNotes() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------
