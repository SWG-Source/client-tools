//
// FormBoundaryPolygon.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormBoundaryPolygon.h"

#include "DialogControlPointEditor.h"
#include "sharedTerrain/Boundary.h"
#include "sharedUtility/FileName.h"
#include "RecentDirectory.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormBoundaryPolygon, FormLayerItem)

//-------------------------------------------------------------------

FormBoundaryPolygon::FormBoundaryPolygon() : 
	FormLayerItem(FormBoundaryPolygon::IDD),
	boundary (0),

	//-- widgets
	m_featherFunction (),
	m_name (),
	m_featherDistance (false),
	m_isWaterTable (FALSE),
	m_waterHeight (true),
	m_waterShader (),
	m_waterShaderSize (false),
	m_waterType ()
{
	//{{AFX_DATA_INIT(FormBoundaryPolygon)
	m_name = _T("");
	m_isWaterTable = FALSE;
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------

FormBoundaryPolygon::~FormBoundaryPolygon()
{
	boundary = 0;
}

//-------------------------------------------------------------------

void FormBoundaryPolygon::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormBoundaryPolygon)
	DDX_Control(pDX, IDC_EDIT_SHADER, m_waterShader);
	DDX_Control(pDX, IDC_BUTTON_SHADER_BROWSE, m_browseButton);
	DDX_Control(pDX, IDC_FEATHER_DISTANCE, m_featherDistance);
	DDX_Control(pDX, IDC_EDIT_SHADERSIZE, m_waterShaderSize);
	DDX_Control(pDX, IDC_EDIT_HEIGHT, m_waterHeight);
	DDX_Control(pDX, IDC_FEATHER_FUNCTION, m_featherFunction);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Check(pDX, IDC_CHECK_ISWATERTABLE, m_isWaterTable);
	DDX_Control(pDX, IDC_WATER_TYPE2, m_waterType);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormBoundaryPolygon, PropertyView)
	//{{AFX_MSG_MAP(FormBoundaryPolygon)
	ON_CBN_SELCHANGE(IDC_FEATHER_FUNCTION, OnSelchangeFeatherFunction)
	ON_BN_CLICKED(IDC_BUTTON_SHADER_BROWSE, OnButtonShaderBrowse)
	ON_BN_CLICKED(IDC_CHECK_ISWATERTABLE, OnCheckIswatertable)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CONTROL_POINT_LIST, OnButtonEditControlPointList)
	ON_CBN_SELCHANGE(IDC_WATER_TYPE2, OnSelchangeWaterType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormBoundaryPolygon::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormBoundaryPolygon::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormBoundaryPolygon::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* const flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	boundary = dynamic_cast<BoundaryPolygon*> (flivd->item->layerItem);
	NOT_NULL (boundary);
}

//-------------------------------------------------------------------

void FormBoundaryPolygon::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (boundary);

	m_name = boundary->getName();
	IGNORE_RETURN (m_featherFunction.SetCurSel (boundary->getFeatherFunction ()));
	m_featherDistance = boundary->getFeatherDistance ();
	m_isWaterTable = boundary->isLocalWaterTable ();
	m_waterHeight = boundary->getLocalWaterTableHeight ();
	m_waterShader.SetWindowText (boundary->getLocalWaterTableShaderTemplateName ());
	m_waterShaderSize = boundary->getLocalWaterTableShaderSize ();
	IGNORE_RETURN (m_waterType.SetCurSel (boundary->getWaterType ()));

	IGNORE_RETURN (m_waterHeight.EnableWindow (m_isWaterTable ? TRUE : FALSE));
	IGNORE_RETURN (m_waterShader.EnableWindow (m_isWaterTable ? TRUE : FALSE));
	IGNORE_RETURN (m_waterShaderSize.EnableWindow (m_isWaterTable ? TRUE : FALSE));
	IGNORE_RETURN (m_browseButton.EnableWindow (m_isWaterTable ? TRUE : FALSE));
	IGNORE_RETURN (m_waterType.EnableWindow (m_isWaterTable ? TRUE : FALSE));

	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormBoundaryPolygon::OnSelchangeWaterType() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormBoundaryPolygon::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormBoundaryPolygon::HasChanged () const
{
	CString waterShader;
	m_waterShader.GetWindowText (waterShader);

	return 
		(m_featherFunction.GetCurSel () != boundary->getFeatherFunction ()) ||
		(m_featherDistance              != boundary->getFeatherDistance ()) ||
		((m_isWaterTable == TRUE)       != boundary->isLocalWaterTable ()) ||
		(m_waterHeight                  != boundary->getLocalWaterTableHeight ()) ||
		(waterShader                    != (boundary->getLocalWaterTableShaderTemplateName () ? boundary->getLocalWaterTableShaderTemplateName () : "")) ||
		(m_waterShaderSize              != boundary->getLocalWaterTableShaderSize ());
}

//-------------------------------------------------------------------

void FormBoundaryPolygon::ApplyChanges ()
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (boundary);

		CString waterShader;
		m_waterShader.GetWindowText (waterShader);

		boundary->setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (m_featherFunction.GetCurSel ()));
		boundary->setFeatherDistance (m_featherDistance);
		boundary->setLocalWaterTable (m_isWaterTable == TRUE);
		boundary->setLocalWaterTableHeight (m_waterHeight);
		boundary->setLocalWaterTableShaderTemplateName (waterShader);
		boundary->setLocalWaterTableShaderSize (m_waterShaderSize);
		boundary->setWaterType (static_cast<TerrainGeneratorWaterType> (m_waterType.GetCurSel ()));


		IGNORE_RETURN (m_waterHeight.EnableWindow (m_isWaterTable ? TRUE : FALSE));
		IGNORE_RETURN (m_waterShader.EnableWindow (m_isWaterTable ? TRUE : FALSE));
		IGNORE_RETURN (m_waterShaderSize.EnableWindow (m_isWaterTable ? TRUE : FALSE));
		IGNORE_RETURN (m_browseButton.EnableWindow (m_isWaterTable ? TRUE : FALSE));
		IGNORE_RETURN (m_waterType.EnableWindow (m_isWaterTable ? TRUE : FALSE));

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormBoundaryPolygon::OnSelchangeFeatherFunction() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormBoundaryPolygon::OnButtonShaderBrowse() 
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

void FormBoundaryPolygon::OnCheckIswatertable() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormBoundaryPolygon::OnButtonEditControlPointList() 
{
	DialogControlPointEditor dlg (boundary->getPointList ());
	if (dlg.DoModal ())
	{
		boundary->copyPointList (dlg.getPointList ());
		ApplyChanges ();
	}
}

//-------------------------------------------------------------------

