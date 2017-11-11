//
// FormAffectorFloraDynamicConstant.cpp
// asommers
//
// copyright 2001, sony online enterainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormAffectorFloraDynamicConstant.h"

#include "sharedTerrain/AffectorFloraDynamic.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormAffectorFloraDynamicConstant, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormAffectorFloraDynamicConstant::FormAffectorFloraDynamicConstant() : 
	FormLayerItem(FormAffectorFloraDynamicConstant::IDD),
	affector (0),

	//-- widgets
	m_editDensity (this),
	m_sliderDensity (),
	m_addButton (),
	m_removeButton (),
	m_familyCtl (),
	m_removeAllRadial (FALSE),
	m_operation (0),
	m_densityOverride (FALSE),
	m_name (),
	m_type ()
{
	//{{AFX_DATA_INIT(FormAffectorFloraDynamicConstant)
	m_removeAllRadial = FALSE;
	m_operation = -1;
	m_densityOverride = FALSE;
	m_name = _T("");
	m_type = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------

FormAffectorFloraDynamicConstant::~FormAffectorFloraDynamicConstant()
{
	affector = 0;
}

//-------------------------------------------------------------------

void FormAffectorFloraDynamicConstant::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormAffectorFloraDynamicConstant)
	DDX_Control(pDX, IDC_SLIDER_DENSITY, m_sliderDensity);
	DDX_Control(pDX, IDC_EDIT_DENSITY, m_editDensity);
	DDX_Control(pDX, IDC_ADD, m_addButton);
	DDX_Control(pDX, IDC_RADIO1, m_removeButton);
	DDX_Control(pDX, IDC_FAMILY, m_familyCtl);
	DDX_Check(pDX, IDC_CHECK_REMOVE_ALL_RADIAL, m_removeAllRadial);
	DDX_Radio(pDX, IDC_ADD, m_operation);
	DDX_Check(pDX, IDC_CHECK_DENSITYOVERRIDE, m_densityOverride);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Text(pDX, IDC_STATIC_TYPE, m_type);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormAffectorFloraDynamicConstant, PropertyView)
	//{{AFX_MSG_MAP(FormAffectorFloraDynamicConstant)
	ON_CBN_SELCHANGE(IDC_FAMILY, OnSelchangefamily)
	ON_BN_CLICKED(IDC_ADD, Onadd)
	ON_BN_CLICKED(IDC_RADIO1, OnRemove)
	ON_BN_CLICKED(IDC_CHECK_REMOVE_ALL_RADIAL, OnCheckRemoveAllRadial)
	ON_BN_CLICKED(IDC_CHECK_DENSITYOVERRIDE, OnCheckDensityoverride)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormAffectorFloraDynamicConstant::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormAffectorFloraDynamicConstant::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormAffectorFloraDynamicConstant::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	affector = dynamic_cast<AffectorFloraDynamic*> (flivd->item->layerItem);
	NOT_NULL (affector);
}

//-------------------------------------------------------------------

void FormAffectorFloraDynamicConstant::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (affector);

	m_type = dynamic_cast<AffectorFloraDynamicNearConstant *> (affector) ? "AffectorFloraDynamicNearConstant" : "AffectorFloraDynamicFarConstant";
	m_name = affector->getName ();

	TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	doc->BuildRadialFamilyDropList (m_familyCtl, affector->getFamilyId ());

	m_densityOverride        = affector->getDensityOverride () ? TRUE : FALSE;
	m_editDensity.LinkSmartSliderCtrl (&m_sliderDensity);
	m_editDensity.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editDensity.SetValue (affector->getDensityOverrideDensity ()));

	m_removeAllRadial = affector->getRemoveAll ();

	IGNORE_RETURN (m_addButton.EnableWindow (!m_removeAllRadial ? TRUE : FALSE));
	IGNORE_RETURN (m_removeButton.EnableWindow (!m_removeAllRadial ? TRUE : FALSE));
	IGNORE_RETURN (m_familyCtl.EnableWindow (!m_removeAllRadial ? TRUE : FALSE));
	IGNORE_RETURN (m_editDensity.EnableWindow (m_densityOverride));

	m_operation = affector->getOperation () == TGO_add ? 0 : 1;

	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormAffectorFloraDynamicConstant::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormAffectorFloraDynamicConstant::HasChanged () const
{
	return true;
}

//-------------------------------------------------------------------

void FormAffectorFloraDynamicConstant::ApplyChanges (void)
{
	if (!m_initialized)
		return;

	// TODO: Add your control notification handler code here
	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (affector);

		affector->setFamilyId (static_cast<int> (m_familyCtl.GetItemData (m_familyCtl.GetCurSel ())));
		affector->setOperation (m_operation == 0 ? TGO_add : TGO_replace);
		affector->setRemoveAll (m_removeAllRadial == TRUE);
		affector->setDensityOverride (m_densityOverride == TRUE);
		affector->setDensityOverrideDensity (m_editDensity.GetValueFloat ());

		IGNORE_RETURN (m_addButton.EnableWindow (!m_removeAllRadial ? TRUE : FALSE));
		IGNORE_RETURN (m_removeButton.EnableWindow (!m_removeAllRadial ? TRUE : FALSE));
		IGNORE_RETURN (m_familyCtl.EnableWindow (!m_removeAllRadial ? TRUE : FALSE));
		IGNORE_RETURN (m_editDensity.EnableWindow (m_densityOverride));

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormAffectorFloraDynamicConstant::OnSelchangefamily() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorFloraDynamicConstant::Onadd() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorFloraDynamicConstant::OnRemove() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorFloraDynamicConstant::OnCheckRemoveAllRadial() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorFloraDynamicConstant::OnCheckDensityoverride() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

