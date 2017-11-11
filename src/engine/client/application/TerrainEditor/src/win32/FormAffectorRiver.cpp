//
// FormAffectorRiver.cpp
// asommers
// 
// copyright 2000, sony online entertainment

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormAffectorRiver.h"

#include "DialogControlPointEditor.h"
#include "sharedTerrain/AffectorRiver.h"
#include "sharedUtility/FileName.h"
#include "RecentDirectory.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormAffectorRiver, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormAffectorRiver::FormAffectorRiver() : 
	FormLayerItem(FormAffectorRiver::IDD),
	affector (0),
	m_editFeatherDistance (this),
	m_sliderFeatherDistance (),

	//-- widgets
	m_waterDepth (false),
	m_waterWidth (false),
	m_trenchDepth (false),
	m_featherFunction (),
	m_width (false),
	m_waterShader (),
	m_waterShaderSize (false),
	m_velocity (false),
	m_waterType (),
	m_name ()
{
	//{{AFX_DATA_INIT(FormAffectorRiver)
	m_name = _T("");
	m_hasWater = FALSE;
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------
	
FormAffectorRiver::~FormAffectorRiver()
{
	affector = 0;
}

//-------------------------------------------------------------------

void FormAffectorRiver::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormAffectorRiver)
	DDX_Control(pDX, IDC_FAMILY_BOTTOM, m_familyBottom);
	DDX_Control(pDX, IDC_FAMILY_BANK, m_familyBank);
	DDX_Control(pDX, IDC_EDIT_SHADER, m_waterShader);
	DDX_Control(pDX, IDC_EDIT_WATERDEPTH, m_waterDepth);
	DDX_Control(pDX, IDC_EDIT_WATERWIDTH, m_waterWidth);
	DDX_Control(pDX, IDC_EDIT_TRENCHDEPTH, m_trenchDepth);
	DDX_Control(pDX, IDC_BUTTON_SHADER_BROWSE, m_browse);
	DDX_Control(pDX, IDC_EDIT_WIDTH2, m_width);
	DDX_Control(pDX, IDC_EDIT_VELOCITY, m_velocity);
	DDX_Control(pDX, IDC_EDIT_SHADERSIZE, m_waterShaderSize);
	DDX_Control(pDX, IDC_SLIDER_FEATHERDISTANCE, m_sliderFeatherDistance);
	DDX_Control(pDX, IDC_FEATHER_DISTANCE, m_editFeatherDistance);
	DDX_Control(pDX, IDC_FEATHER_FUNCTION, m_featherFunction);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Check(pDX, IDC_CHECK_HASWATER, m_hasWater);
	DDX_Control(pDX, IDC_WATER_TYPE2,m_waterType);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormAffectorRiver, PropertyView)
	//{{AFX_MSG_MAP(FormAffectorRiver)
	ON_CBN_SELCHANGE(IDC_FEATHER_FUNCTION, OnSelchangeFeatherFunction)
	ON_BN_CLICKED(IDC_BUTTON_SHADER_BROWSE, OnButtonShaderBrowse)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK_HASWATER, OnCheckHaswater)
	ON_CBN_SELCHANGE(IDC_FAMILY_BANK, OnSelchangeFamilyBank)
	ON_CBN_SELCHANGE(IDC_FAMILY_BOTTOM, OnSelchangeFamilyBottom)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CONTROL_POINT_LIST, OnButtonEditControlPointList)
	ON_CBN_SELCHANGE(IDC_WATER_TYPE2, OnSelchangeWaterType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormAffectorRiver::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormAffectorRiver::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormAffectorRiver::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	affector = dynamic_cast<AffectorRiver*>(flivd->item->layerItem);
	NOT_NULL (affector);
}

//-------------------------------------------------------------------

void FormAffectorRiver::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (affector);

	m_name   = affector->getName ();
	m_width  = affector->getWidth ();
	IGNORE_RETURN (m_featherFunction.SetCurSel (affector->getFeatherFunction ()));
	m_editFeatherDistance.LinkSmartSliderCtrl (&m_sliderFeatherDistance);
	m_editFeatherDistance.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editFeatherDistance.SetValue (affector->getFeatherDistance ()));

	TerrainEditorDoc* const doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	doc->BuildShaderFamilyDropList (m_familyBank, affector->getBankFamilyId ());
	doc->BuildShaderFamilyDropList (m_familyBottom, affector->getBottomFamilyId ());

	m_trenchDepth     = affector->getTrenchDepth ();
	m_waterDepth      = affector->getLocalWaterTableDepth ();
	m_waterWidth      = affector->getLocalWaterTableWidth ();
	m_velocity        = affector->getVelocity ();
	m_waterShader.SetWindowText (affector->getLocalWaterTableShaderTemplateName ());
	m_waterShaderSize = affector->getLocalWaterTableShaderSize ();
	m_hasWater        = affector->getHasLocalWaterTable ();
	IGNORE_RETURN (m_waterType.SetCurSel (affector->getWaterType ()));


	IGNORE_RETURN (m_browse.EnableWindow (m_hasWater ? TRUE : FALSE));
	IGNORE_RETURN (m_waterShader.EnableWindow (m_hasWater ? TRUE : FALSE));
	IGNORE_RETURN (m_waterShaderSize.EnableWindow (m_hasWater ? TRUE : FALSE));
	IGNORE_RETURN (m_waterDepth.EnableWindow (m_hasWater ? TRUE : FALSE));
	IGNORE_RETURN (m_waterWidth.EnableWindow (m_hasWater ? TRUE : FALSE));
	IGNORE_RETURN (m_waterType.EnableWindow (m_hasWater ? TRUE : FALSE));

	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormAffectorRiver::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormAffectorRiver::HasChanged () const
{
	return true;
}

//-------------------------------------------------------------------

void FormAffectorRiver::OnSelchangeWaterType() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorRiver::ApplyChanges ()
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (affector);

		affector->setWidth (m_width);
		affector->setBankFamilyId (static_cast<int> (m_familyBank.GetItemData (m_familyBank.GetCurSel ())));
		affector->setBottomFamilyId (static_cast<int> (m_familyBottom.GetItemData (m_familyBottom.GetCurSel ())));
		affector->setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (m_featherFunction.GetCurSel ()));
		affector->setFeatherDistance (m_editFeatherDistance.GetValueFloat ());
		affector->setHasLocalWaterTable (m_hasWater == TRUE);

		CString waterShader;
		m_waterShader.GetWindowText (waterShader);
		affector->setLocalWaterTableShaderTemplateName (waterShader);
		affector->setLocalWaterTableShaderSize (m_waterShaderSize);
		affector->setTrenchDepth (m_trenchDepth);
		affector->setLocalWaterTableDepth (m_waterDepth);
		affector->setLocalWaterTableWidth (m_waterWidth);
		affector->setVelocity (m_velocity);
		affector->setWaterType (static_cast<TerrainGeneratorWaterType> (m_waterType.GetCurSel ()));


		IGNORE_RETURN (m_browse.EnableWindow (m_hasWater ? TRUE : FALSE));
		IGNORE_RETURN (m_waterShader.EnableWindow (m_hasWater ? TRUE : FALSE));
		IGNORE_RETURN (m_waterShaderSize.EnableWindow (m_hasWater ? TRUE : FALSE));
		IGNORE_RETURN (m_waterDepth.EnableWindow (m_hasWater ? TRUE : FALSE));
		IGNORE_RETURN (m_waterWidth.EnableWindow (m_hasWater ? TRUE : FALSE));
		IGNORE_RETURN (m_waterType.EnableWindow (m_hasWater ? TRUE : FALSE));

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormAffectorRiver::OnButtonShaderBrowse() 
{
	IGNORE_RETURN (UpdateData (true));

	CFileDialog dlg (true, "*.sht", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("ShaderTemplate_Water");
	dlg.m_ofn.lpstrTitle      = "Select Shader Template File";

	if (dlg.DoModal () == IDOK)
	{
		IGNORE_RETURN (RecentDirectory::update ("ShaderTemplate_Water", dlg.GetPathName ()));

		FileName filename (dlg.GetPathName ());
		filename.stripPathAndExt ();

		m_waterShader.SetWindowText (filename);

		IGNORE_RETURN (UpdateData (false));

		ApplyChanges ();
	}
}

//-------------------------------------------------------------------

void FormAffectorRiver::OnSelchangeFeatherFunction() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorRiver::OnCheckHaswater() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorRiver::OnSelchangeFamilyBank() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorRiver::OnSelchangeFamilyBottom() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorRiver::OnButtonEditControlPointList() 
{
	DialogControlPointEditor dlg (affector->getPointList ());
	if (dlg.DoModal ())
	{
		affector->copyPointList (dlg.getPointList ());
		ApplyChanges ();
	}
}

//-------------------------------------------------------------------

