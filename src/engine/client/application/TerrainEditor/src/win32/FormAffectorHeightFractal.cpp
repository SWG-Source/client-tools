//
// FormAffectorHeightFractal.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormAffectorHeightFractal.h"

#include "sharedTerrain/AffectorHeight.h"
#include "CDib.h"
#include "FractalPreviewFrame.h"
#include "MainFrame.h"
#include "TerrainEditor.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormAffectorHeightFractal, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormAffectorHeightFractal::FormAffectorHeightFractal() : 
	FormLayerItem(FormAffectorHeightFractal::IDD),
	m_affector (0),

	//-- widgets
	m_familyCtrl (),
	m_scaleY (true),
	m_operation (),
	m_name ()
{
	//{{AFX_DATA_INIT(FormAffectorHeightFractal)
	m_name = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------

FormAffectorHeightFractal::~FormAffectorHeightFractal()
{
	m_affector = 0;
}

//-------------------------------------------------------------------

void FormAffectorHeightFractal::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormAffectorHeightFractal)
	DDX_Control(pDX, IDC_FAMILY, m_familyCtrl);
	DDX_Control(pDX, IDC_SCALEY, m_scaleY);
	DDX_Control(pDX, IDC_OPERATION, m_operation);
	DDX_Text(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormAffectorHeightFractal, PropertyView)
	//{{AFX_MSG_MAP(FormAffectorHeightFractal)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_OPERATION, OnSelchangeOperation)
	ON_CBN_SELCHANGE(IDC_FAMILY, OnSelchangeFamily)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormAffectorHeightFractal::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormAffectorHeightFractal::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormAffectorHeightFractal::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	m_affector = dynamic_cast<AffectorHeightFractal*> (flivd->item->layerItem);
	NOT_NULL (m_affector);
}

//-------------------------------------------------------------------

void FormAffectorHeightFractal::OnInitialUpdate() 
{
	PropertyView::OnInitialUpdate();

	NOT_NULL (m_affector);

	m_name = m_affector->getName ();

	TerrainEditorDoc* const doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	doc->BuildFractalFamilyDropList (m_familyCtrl, m_affector->getFamilyId ());

	IGNORE_RETURN (m_operation.SetCurSel (m_affector->getOperation ()));

	m_scaleY = m_affector->getScaleY ();

	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;

	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorHeightFractal::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormAffectorHeightFractal::HasChanged () const
{
	return true;
}

//-------------------------------------------------------------------

void FormAffectorHeightFractal::ApplyChanges ()
{
	if (!m_initialized)
		return;

	//-- update the controls/data
	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (m_affector);

		//-- update operation
		m_affector->setFamilyId (static_cast<int> (m_familyCtrl.GetItemData (m_familyCtrl.GetCurSel ())));
		m_affector->setScaleY (m_scaleY);
		m_affector->setOperation (static_cast<TerrainGeneratorOperation> (m_operation.GetCurSel ()));

		safe_cast<TerrainEditorApp*> (AfxGetApp ())->showFractalPreview ();
		GetParentFrame ()->ActivateFrame ();

		TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
		NOT_NULL (doc);
		
		const MultiFractal* multiFractal = doc->getTerrainGenerator ()->getFractalGroup ().getFamilyMultiFractal (m_affector->getFamilyId ());
		NOT_NULL (multiFractal);

		if (doc->getFractalPreviewFrame ())
			doc->getFractalPreviewFrame ()->updateBitmap (*multiFractal);

		//-- tell the document we've modified data
		GetDocument ()->SetModifiedFlag (true);
		GetDocument ()->UpdateAllViews (this);
	}
}

//-------------------------------------------------------------------

void FormAffectorHeightFractal::OnSelchangeOperation() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorHeightFractal::OnSelchangeFamily() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

