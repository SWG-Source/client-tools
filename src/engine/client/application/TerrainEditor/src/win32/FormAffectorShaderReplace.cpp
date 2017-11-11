//
// FormAffectorShaderReplace.cpp
// asommers
// 
// copyright 2000, sony online entertainment

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormAffectorShaderReplace.h"

#include "sharedTerrain/AffectorShader.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormAffectorShaderReplace, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormAffectorShaderReplace::FormAffectorShaderReplace() : 
	FormLayerItem(FormAffectorShaderReplace::IDD),
	affector (0),

	//-- widgets
	m_editFeatherClamp (this),
	m_sliderFeatherClamp (),
	m_featherClampOverride (FALSE),
	m_familySource (),
	m_familyDestination (),
	m_name ()
{
	//{{AFX_DATA_INIT(FormAffectorShaderReplace)
	m_featherClampOverride = FALSE;
	m_name = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------
	
FormAffectorShaderReplace::~FormAffectorShaderReplace()
{
	affector = 0;
}

//-------------------------------------------------------------------

void FormAffectorShaderReplace::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormAffectorShaderReplace)
	DDX_Control(pDX, IDC_SLIDER_FEATHERCLAMP, m_sliderFeatherClamp);
	DDX_Control(pDX, IDC_EDIT_FEATHERCLAMP, m_editFeatherClamp);
	DDX_Check(pDX, IDC_CHECK_FEATHERCLAMPOVERRIDE, m_featherClampOverride);
	DDX_Control(pDX, IDC_FAMILY_SOURCE, m_familySource);
	DDX_Control(pDX, IDC_FAMILY_DESTINATION, m_familyDestination);
	DDX_Text(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormAffectorShaderReplace, PropertyView)
	//{{AFX_MSG_MAP(FormAffectorShaderReplace)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_FAMILY_DESTINATION, OnSelchangeFamilyDestination)
	ON_CBN_SELCHANGE(IDC_FAMILY_SOURCE, OnSelchangeFamilySource)
	ON_BN_CLICKED(IDC_CHECK_FEATHERCLAMPOVERRIDE, OnCheckFeatherClampoverride)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormAffectorShaderReplace::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormAffectorShaderReplace::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormAffectorShaderReplace::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	affector = dynamic_cast<AffectorShaderReplace*>(flivd->item->layerItem);
	NOT_NULL (affector);
}

//-------------------------------------------------------------------

void FormAffectorShaderReplace::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (affector);

	TerrainEditorDoc* const doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	doc->BuildShaderFamilyDropList (m_familySource, affector->getSourceFamilyId ());
	doc->BuildShaderFamilyDropList (m_familyDestination, affector->getDestinationFamilyId ());

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

void FormAffectorShaderReplace::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormAffectorShaderReplace::HasChanged () const
{
	return 
		(static_cast<int> (m_familySource.GetItemData (m_familySource.GetCurSel ())) != affector->getSourceFamilyId ()) ||
		(static_cast<int> (m_familyDestination.GetItemData (m_familyDestination.GetCurSel ())) != affector->getDestinationFamilyId ()) ||
		((m_featherClampOverride == TRUE)    != affector->getUseFeatherClampOverride ()) ||
		(m_editFeatherClamp.GetValueFloat () != affector->getFeatherClampOverride ());
}

//-------------------------------------------------------------------

void FormAffectorShaderReplace::ApplyChanges (void)
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (affector);

		affector->setSourceFamilyId (static_cast<int> (m_familySource.GetItemData (m_familySource.GetCurSel ())));
		affector->setDestinationFamilyId (static_cast<int> (m_familyDestination.GetItemData (m_familyDestination.GetCurSel ())));
		affector->setUseFeatherClampOverride (m_featherClampOverride == TRUE);
		affector->setFeatherClampOverride (m_editFeatherClamp.GetValueFloat ());

		IGNORE_RETURN (m_editFeatherClamp.EnableWindow (m_featherClampOverride));

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormAffectorShaderReplace::OnSelchangeFamilyDestination() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorShaderReplace::OnSelchangeFamilySource() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorShaderReplace::OnCheckFeatherClampoverride() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

