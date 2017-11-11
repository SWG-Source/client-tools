//
// FormFilterShader.cpp
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormFilterShader.h"

#include "sharedTerrain/Filter.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormFilterShader, FormLayerItem)

//-------------------------------------------------------------------

FormFilterShader::FormFilterShader() : 
	FormLayerItem(FormFilterShader::IDD),
	m_filter (0),

	//-- widgets
	m_familyCtl (),
	m_name ()
{
	//{{AFX_DATA_INIT(FormFilterShader)
	m_name = _T("");
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------
	
FormFilterShader::~FormFilterShader()
{
	m_filter = 0;
}

//-------------------------------------------------------------------

void FormFilterShader::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormFilterShader)
	DDX_Control(pDX, IDC_FAMILY, m_familyCtl);
	DDX_Text(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormFilterShader, PropertyView)
	//{{AFX_MSG_MAP(FormFilterShader)
	ON_CBN_SELCHANGE(IDC_FAMILY, OnSelchangefamily)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormFilterShader::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormFilterShader::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormFilterShader::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	m_filter = dynamic_cast<FilterShader*> (flivd->item->layerItem);
	NOT_NULL (m_filter);
}

//-------------------------------------------------------------------

void FormFilterShader::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (m_filter);

	m_name = m_filter->getName ();

	TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	doc->BuildShaderFamilyDropList (m_familyCtl, m_filter->getFamilyId ());

	IGNORE_RETURN (UpdateData (FALSE));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormFilterShader::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormFilterShader::HasChanged () const
{
	return 
		(static_cast<int> (m_familyCtl.GetItemData (m_familyCtl.GetCurSel ())) != m_filter->getFamilyId ());
}

//-------------------------------------------------------------------

void FormFilterShader::ApplyChanges (void)
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (TRUE));

	if (HasChanged ())
	{
		NOT_NULL (m_filter);

		m_filter->setFamilyId (static_cast<int> (m_familyCtl.GetItemData (m_familyCtl.GetCurSel ())));

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormFilterShader::OnSelchangefamily() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------
