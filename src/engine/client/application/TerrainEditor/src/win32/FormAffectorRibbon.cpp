//
// FormAffectorRibbon.cpp
//
// copyright 2005 Sony Online Entertainment

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormAffectorRibbon.h"

#include "DialogControlPointEditor.h"
#include "sharedTerrain/AffectorRibbon.h"
#include "sharedUtility/FileName.h"
#include "RecentDirectory.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormAffectorRibbon, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormAffectorRibbon::FormAffectorRibbon() :
	FormLayerItem(FormAffectorRibbon::IDD),
	affector (0),
	//-- widgets
	m_width (false),
	m_capWidth (false),
	m_waterShaderSize (false),
	m_velocity (false),
	m_waterShader (),
	m_waterType (),
	m_sliderFeatherDistanceTerrainShader(),
	m_featherFunctionTerrainShader(),
	m_editFeatherDistanceTerrainShader(this),
	m_terrainShaderFamilyCtl(),
	m_name ()
{
	//{{AFX_DATA_INIT(FormAffectorRibbon)
	m_name = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------

FormAffectorRibbon::~FormAffectorRibbon()
{
	affector = 0;
}

//-------------------------------------------------------------------

void FormAffectorRibbon::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormAffectorRibbon)
	DDX_Control(pDX, IDC_EDIT_WIDTH2, m_width);
	DDX_Control(pDX, IDC_EDIT_CAP_WIDTH, m_capWidth);
	DDX_Control(pDX, IDC_EDIT_WATER_SHADER_SIZE, m_waterShaderSize);
	DDX_Control(pDX, IDC_EDIT_VELOCITY, m_velocity);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Control(pDX, IDC_BUTTON_WATER_SHADER_BROWSE, m_browse);
	DDX_Control(pDX, IDC_EDIT_WATER_SHADER, m_waterShader);
	DDX_Control(pDX, IDC_WATER_TYPE, m_waterType);
	DDX_Control(pDX, IDC_SLIDER_FEATHER_DISTANCE_TERRAIN_SHADER, m_sliderFeatherDistanceTerrainShader);
	DDX_Control(pDX, IDC_FEATHER_FUNCTION_TERRAIN_SHADER, m_featherFunctionTerrainShader);
	DDX_Control(pDX, IDC_FEATHER_DISTANCE_TERRAIN_SHADER, m_editFeatherDistanceTerrainShader);
	DDX_Control(pDX, IDC_TERRAIN_SHADER_FAMILY, m_terrainShaderFamilyCtl);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------


//lint -save -e1924

BEGIN_MESSAGE_MAP(FormAffectorRibbon, PropertyView)
	//{{AFX_MSG_MAP(FormAffectorRibbon)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_WATER_SHADER_BROWSE, OnButtonWaterShaderBrowse)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CONTROL_POINT_LIST, OnButtonEditControlPointList)
	ON_EN_CHANGE(IDC_EDIT_WIDTH2, ApplyChanges)
	ON_EN_CHANGE(IDC_EDIT_CAP_WIDTH, ApplyChanges)
	ON_CBN_SELCHANGE(IDC_WATER_TYPE, OnSelchangeWaterType)
	ON_CBN_SELCHANGE(IDC_TERRAIN_SHADER_FAMILY, OnSelchangeTerrainShaderFamily)
	ON_CBN_SELCHANGE(IDC_FEATHER_FUNCTIONSHADER, OnSelchangeFeatherFunctionTerrainShader)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormAffectorRibbon::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormAffectorRibbon::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormAffectorRibbon::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	affector = dynamic_cast<AffectorRibbon*>(flivd->item->layerItem);
	NOT_NULL (affector);
}

//-------------------------------------------------------------------

void FormAffectorRibbon::OnInitialUpdate()
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (affector);

	m_name   = affector->getName ();
	m_width  = affector->getWidth ();
	m_capWidth  = affector->getCapWidth ();
	m_waterShaderSize = affector->getWaterShaderSize ();
	m_velocity = affector->getVelocity ();
	m_waterShader.SetWindowText (affector->getRibbonWaterShaderTemplateName ());
	m_editFeatherDistanceTerrainShader.LinkSmartSliderCtrl (&m_sliderFeatherDistanceTerrainShader);
	m_editFeatherDistanceTerrainShader.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_featherFunctionTerrainShader.SetCurSel (affector->getFeatherFunctionTerrainShader ()));
	IGNORE_RETURN (m_editFeatherDistanceTerrainShader.SetValue (affector->getFeatherDistanceTerrainShader ()));
	if(affector->getPointList().size() != affector->getHeightList().size())
	{
		affector->createInitialHeightList();
	}
	IGNORE_RETURN (m_waterType.SetCurSel (affector->getWaterType ()));
	
	TerrainEditorDoc* const doc = static_cast<TerrainEditorDoc*> (GetDocument ());
	doc->BuildShaderFamilyDropList (m_terrainShaderFamilyCtl, affector->getTerrainShaderFamilyId ());

	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormAffectorRibbon::OnDestroy()
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormAffectorRibbon::HasChanged () const
{
	return true;
}

//-------------------------------------------------------------------

void FormAffectorRibbon::OnSelchangeWaterType() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorRibbon::OnSelchangeFeatherFunctionTerrainShader() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorRibbon::OnSelchangeTerrainShaderFamily() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorRibbon::ApplyChanges ()
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (affector);

		CString waterShader;
		m_waterShader.GetWindowText (waterShader);
		affector->setRibbonWaterShaderTemplateName (waterShader);

		affector->setWidth (m_width);
		affector->setTerrainShaderFamilyId (static_cast<int> (m_terrainShaderFamilyCtl.GetItemData (m_terrainShaderFamilyCtl.GetCurSel ())));
		float old_capWidth = affector->getCapWidth();
		affector->setCapWidth (m_capWidth);
		affector->setWaterShaderSize (m_waterShaderSize);
		affector->setVelocity (m_velocity);
		affector->setWaterType (static_cast<TerrainGeneratorWaterType> (m_waterType.GetCurSel ()));
		affector->setFeatherFunctionTerrainShader (static_cast<TerrainGeneratorFeatherFunction> (m_featherFunctionTerrainShader.GetCurSel ()));
		affector->setFeatherDistanceTerrainShader (m_editFeatherDistanceTerrainShader.GetValueFloat ());
		if(m_capWidth != old_capWidth)
		{
			affector->generateEndCapPointList();
		}
		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------


void FormAffectorRibbon::OnButtonEditControlPointList()
{
	DialogControlPointEditor dlg (affector->getPointList (),affector->getHeightList());
	if (dlg.DoModal ())
	{
		affector->copyPointList (dlg.getPointList ());
		affector->copyHeightList (dlg.getHeightList ());
		affector->generateEndCapPointList();
		ApplyChanges ();
	}
}

//-------------------------------------------------------------------

void FormAffectorRibbon::OnButtonWaterShaderBrowse() 
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
