//
// FormAffectorHeightConstant.cpp
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormAffectorHeightConstant.h"

#include "sharedTerrain/AffectorHeight.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormAffectorHeightConstant, FormLayerItem)

//-------------------------------------------------------------------

FormAffectorHeightConstant::FormAffectorHeightConstant() : 
	FormLayerItem(FormAffectorHeightConstant::IDD),
	affector (0),

	//-- widgets
	m_value (true),
	m_name (),
	m_operation (0)
{
	//{{AFX_DATA_INIT(FormAffectorHeightConstant)
	m_name = _T("");
	m_operation = -1;
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------
	
FormAffectorHeightConstant::~FormAffectorHeightConstant()
{
	affector = 0;
}

//-------------------------------------------------------------------

void FormAffectorHeightConstant::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormAffectorHeightConstant)
	DDX_Control(pDX, IDC_VALUE, m_value);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_CBIndex(pDX, IDC_OPERATION, m_operation);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormAffectorHeightConstant, FormLayerItem)
	//{{AFX_MSG_MAP(FormAffectorHeightConstant)
	ON_CBN_SELCHANGE(IDC_OPERATION, OnSelchangeOperation)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormAffectorHeightConstant::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormAffectorHeightConstant::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormAffectorHeightConstant::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	affector = dynamic_cast<AffectorHeightConstant*> (flivd->item->layerItem);
	NOT_NULL (affector);
}

//-------------------------------------------------------------------

void FormAffectorHeightConstant::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (affector);

	m_name      = affector->getName ();
	m_value     = affector->getHeight ();
	m_operation = static_cast<int> (affector->getOperation ());

	IGNORE_RETURN (UpdateData(false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormAffectorHeightConstant::OnDestroy() 
{
	ApplyChanges ();

	FormLayerItem::OnDestroy();
}

//-------------------------------------------------------------------

bool FormAffectorHeightConstant::HasChanged () const
{
	return true;
}

//-------------------------------------------------------------------

void FormAffectorHeightConstant::ApplyChanges ()
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (affector);

		affector->setHeight (m_value);
		affector->setOperation (static_cast<TerrainGeneratorOperation> (m_operation));

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormAffectorHeightConstant::OnSelchangeOperation() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------


