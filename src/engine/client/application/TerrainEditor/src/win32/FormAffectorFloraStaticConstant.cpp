//
// FormAffectorFloraStaticConstant.cpp
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormAffectorFloraStaticConstant.h"

#include "sharedTerrain/AffectorFloraStatic.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormAffectorFloraStaticConstant, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormAffectorFloraStaticConstant::FormAffectorFloraStaticConstant() : 
	FormLayerItem(FormAffectorFloraStaticConstant::IDD),
	affector (0),

	//-- widgets
	m_editDensity (this),
	m_sliderDensity (),
	m_addButton (),
	m_removeButton (),
	m_familyCtl (),
	m_removeAllFlora (FALSE),
	m_operation (0),
	m_densityOverride (FALSE),
	m_name (),
	m_type ()
{
	//{{AFX_DATA_INIT(FormAffectorFloraStaticConstant)
	m_removeAllFlora = FALSE;
	m_operation = -1;
	m_densityOverride = FALSE;
	m_name = _T("");
	m_type = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------
	
FormAffectorFloraStaticConstant::~FormAffectorFloraStaticConstant()
{
	affector = 0;
}

//-------------------------------------------------------------------

void FormAffectorFloraStaticConstant::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormAffectorFloraStaticConstant)
	DDX_Control(pDX, IDC_SLIDER_DENSITY, m_sliderDensity);
	DDX_Control(pDX, IDC_EDIT_DENSITY, m_editDensity);
	DDX_Control(pDX, IDC_ADD, m_addButton);
	DDX_Control(pDX, IDC_RADIO1, m_removeButton);
	DDX_Control(pDX, IDC_FAMILY, m_familyCtl);
	DDX_Check(pDX, IDC_CHECK_REMOVE_ALL_FLORA, m_removeAllFlora);
	DDX_Radio(pDX, IDC_ADD, m_operation);
	DDX_Check(pDX, IDC_CHECK_DENSITYOVERRIDE, m_densityOverride);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Text(pDX, IDC_STATIC_TYPE, m_type);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormAffectorFloraStaticConstant, PropertyView)
	//{{AFX_MSG_MAP(FormAffectorFloraStaticConstant)
	ON_CBN_SELCHANGE(IDC_FAMILY, OnSelchangefamily)
	ON_BN_CLICKED(IDC_ADD, Onadd)
	ON_BN_CLICKED(IDC_RADIO1, OnRemove)
	ON_BN_CLICKED(IDC_CHECK_REMOVE_ALL_FLORA, OnCheckRemoveAllFlora)
	ON_BN_CLICKED(IDC_CHECK_DENSITYOVERRIDE, OnCheckDensityoverride)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormAffectorFloraStaticConstant::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormAffectorFloraStaticConstant::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormAffectorFloraStaticConstant::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (affector);

	m_type = dynamic_cast<AffectorFloraStaticCollidableConstant *> (affector) ? "AffectorFloraStaticCollidableConstant" : "AffectorFloraStaticNonCollidableConstant";
	m_name = affector->getName ();

	TerrainEditorDoc* doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	doc->BuildFloraFamilyDropList (m_familyCtl, affector->getFamilyId ());

	m_densityOverride        = affector->getDensityOverride () ? TRUE : FALSE;
	m_editDensity.LinkSmartSliderCtrl (&m_sliderDensity);
	m_editDensity.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editDensity.SetValue (affector->getDensityOverrideDensity ()));

	m_removeAllFlora = affector->getRemoveAll ();

	IGNORE_RETURN (m_addButton.EnableWindow (!m_removeAllFlora ? TRUE : FALSE));
	IGNORE_RETURN (m_removeButton.EnableWindow (!m_removeAllFlora ? TRUE : FALSE));
	IGNORE_RETURN (m_familyCtl.EnableWindow (!m_removeAllFlora ? TRUE : FALSE));
	IGNORE_RETURN (m_editDensity.EnableWindow (m_densityOverride));

	m_operation = affector->getOperation () == TGO_add ? 0 : 1;

	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormAffectorFloraStaticConstant::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	affector = dynamic_cast<AffectorFloraStatic*> (flivd->item->layerItem);
	NOT_NULL (affector);
}

//-------------------------------------------------------------------

void FormAffectorFloraStaticConstant::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormAffectorFloraStaticConstant::HasChanged () const
{
	return true;
}

//-------------------------------------------------------------------

void FormAffectorFloraStaticConstant::ApplyChanges (void)
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
		affector->setRemoveAll (m_removeAllFlora == TRUE);
		affector->setDensityOverride (m_densityOverride == TRUE);
		affector->setDensityOverrideDensity (m_editDensity.GetValueFloat ());

		IGNORE_RETURN (m_addButton.EnableWindow (!m_removeAllFlora ? TRUE : FALSE));
		IGNORE_RETURN (m_removeButton.EnableWindow (!m_removeAllFlora ? TRUE : FALSE));
		IGNORE_RETURN (m_familyCtl.EnableWindow (!m_removeAllFlora ? TRUE : FALSE));
		IGNORE_RETURN (m_editDensity.EnableWindow (m_densityOverride));

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormAffectorFloraStaticConstant::OnSelchangefamily() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorFloraStaticConstant::Onadd() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorFloraStaticConstant::OnRemove() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorFloraStaticConstant::OnCheckRemoveAllFlora() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorFloraStaticConstant::OnCheckDensityoverride() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

