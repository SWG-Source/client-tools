//
// FormAffectorShaderConstant.cpp
// asommers
// 
// copyright 2000, sony online entertainment

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormAffectorShaderConstant.h"

#include "sharedTerrain/AffectorShader.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormAffectorShaderConstant, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormAffectorShaderConstant::FormAffectorShaderConstant() : 
	FormLayerItem(FormAffectorShaderConstant::IDD),
	affector (0),

	//-- widgets
	m_editFeatherClamp (this),
	m_sliderFeatherClamp (),
	m_familyCtl (),
	m_featherClampOverride (FALSE),
	m_name ()
{
	//{{AFX_DATA_INIT(FormAffectorShaderConstant)
	m_featherClampOverride = FALSE;
	m_name = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------
	
FormAffectorShaderConstant::~FormAffectorShaderConstant()
{
	affector = 0;
}

//-------------------------------------------------------------------

void FormAffectorShaderConstant::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormAffectorShaderConstant)
	DDX_Control(pDX, IDC_SLIDER_FEATHERCLAMP, m_sliderFeatherClamp);
	DDX_Control(pDX, IDC_EDIT_FEATHERCLAMP, m_editFeatherClamp);
	DDX_Control(pDX, IDC_FAMILY, m_familyCtl);
	DDX_Check(pDX, IDC_CHECK_FEATHERCLAMPOVERRIDE, m_featherClampOverride);
	DDX_Text(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormAffectorShaderConstant, PropertyView)
	//{{AFX_MSG_MAP(FormAffectorShaderConstant)
	ON_CBN_SELCHANGE(IDC_FAMILY, OnSelchangefamily)
	ON_BN_CLICKED(IDC_CHECK_FEATHERCLAMPOVERRIDE, OnCheckFeatherClampoverride)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormAffectorShaderConstant::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormAffectorShaderConstant::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormAffectorShaderConstant::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	affector = dynamic_cast<AffectorShaderConstant*>(flivd->item->layerItem);
	NOT_NULL (affector);
}

//-------------------------------------------------------------------

void FormAffectorShaderConstant::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (affector);

	TerrainEditorDoc* const doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	doc->BuildShaderFamilyDropList (m_familyCtl, affector->getFamilyId ());

	m_name                   = affector->getName ();
	m_featherClampOverride   = affector->getUseFeatherClampOverride () ? TRUE : FALSE;
	m_editFeatherClamp.LinkSmartSliderCtrl (&m_sliderFeatherClamp);
	m_editFeatherClamp.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editFeatherClamp.SetValue (affector->getFeatherClampOverride ()));

	IGNORE_RETURN (m_editFeatherClamp.EnableWindow (m_featherClampOverride));
	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormAffectorShaderConstant::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormAffectorShaderConstant::HasChanged () const
{
	return 
		(static_cast<int> (m_familyCtl.GetItemData (m_familyCtl.GetCurSel ())) != affector->getFamilyId ()) ||
		((m_featherClampOverride == TRUE)    != affector->getUseFeatherClampOverride ()) ||
		(m_editFeatherClamp.GetValueFloat () != affector->getFeatherClampOverride ());
}

//-------------------------------------------------------------------

void FormAffectorShaderConstant::ApplyChanges (void)
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (affector);

		affector->setFamilyId (static_cast<int> (m_familyCtl.GetItemData (m_familyCtl.GetCurSel ())));
		affector->setUseFeatherClampOverride (m_featherClampOverride == TRUE);
		affector->setFeatherClampOverride (m_editFeatherClamp.GetValueFloat ());

		IGNORE_RETURN (m_editFeatherClamp.EnableWindow (m_featherClampOverride));

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormAffectorShaderConstant::OnSelchangefamily() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorShaderConstant::OnCheckFeatherClampoverride() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

