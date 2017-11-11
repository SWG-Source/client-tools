//
// FormBoundaryRect.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormBoundaryRect.h"

#include "RecentDirectory.h"
#include "sharedTerrain/Boundary.h"
#include "sharedUtility/FileName.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormBoundaryRect, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormBoundaryRect::FormBoundaryRect() : 
	FormLayerItem(FormBoundaryRect::IDD),
	boundary (0),
	m_editFeatherDistance (this),
	m_sliderFeatherDistance (),

	//-- widgets
	m_featherFunction (),
	m_x0 (true),
	m_x1 (true),
	m_z0 (true),
	m_z1 (true),
	m_name (),
	m_isWaterTable (FALSE),
	m_waterHeight (true),
	m_waterShader (),
	m_waterShaderSize (false),
	m_waterType ()
{
	//{{AFX_DATA_INIT(FormBoundaryRect)
	m_name = _T("");
	m_isWaterTable = FALSE;
	m_isLocalGlobalWaterTable = FALSE;
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------
	
FormBoundaryRect::~FormBoundaryRect()
{
	boundary = 0;
}

//-------------------------------------------------------------------

void FormBoundaryRect::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormBoundaryRect)
	DDX_Control(pDX, IDC_CHECK_LOCALGLOBALWATERTABLE, m_localGlobalWaterTable);
	DDX_Control(pDX, IDC_BUTTON_SHADER_BROWSE, m_browseButton);
	DDX_Control(pDX, IDC_EDIT_SHADER, m_waterShader);
	DDX_Control(pDX, IDC_Z1, m_z1);
	DDX_Control(pDX, IDC_Z0, m_z0);
	DDX_Control(pDX, IDC_X1, m_x1);
	DDX_Control(pDX, IDC_X0, m_x0);
	DDX_Control(pDX, IDC_SLIDER_FEATHERDISTANCE, m_sliderFeatherDistance);
	DDX_Control(pDX, IDC_FEATHER_DISTANCE, m_editFeatherDistance);
	DDX_Control(pDX, IDC_FEATHER_FUNCTION, m_featherFunction);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Check(pDX, IDC_CHECK_ISWATERTABLE, m_isWaterTable);
	DDX_Control(pDX, IDC_EDIT_HEIGHT, m_waterHeight);
	DDX_Control(pDX, IDC_EDIT_SHADERSIZE, m_waterShaderSize);
	DDX_Check(pDX, IDC_CHECK_LOCALGLOBALWATERTABLE, m_isLocalGlobalWaterTable);
	DDX_Control(pDX, IDC_WATER_TYPE2, m_waterType);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormBoundaryRect, PropertyView)
	//{{AFX_MSG_MAP(FormBoundaryRect)
	ON_CBN_SELCHANGE(IDC_FEATHER_FUNCTION, OnSelchangeFeatherFunction)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_SHADER_BROWSE, OnButtonShaderBrowse)
	ON_BN_CLICKED(IDC_CHECK_ISWATERTABLE, OnCheckIswatertable)
	ON_BN_CLICKED(IDC_CHECK_LOCALGLOBALWATERTABLE, OnCheckLocalglobalwatertable)
	ON_CBN_SELCHANGE(IDC_WATER_TYPE, OnSelchangeWaterType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormBoundaryRect::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormBoundaryRect::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormBoundaryRect::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* const flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	boundary = dynamic_cast<BoundaryRectangle*> (flivd->item->layerItem);
	NOT_NULL (boundary);
}

//-------------------------------------------------------------------

void FormBoundaryRect::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	update ();

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormBoundaryRect::update (void)
{
	NOT_NULL (boundary);

	m_name = boundary->getName();
	const Rectangle2d rect = boundary->getRectangle ();
	m_x0 = rect.x0;
	m_z0 = rect.y0;
	m_x1 = rect.x1;
	m_z1 = rect.y1;
	IGNORE_RETURN (m_featherFunction.SetCurSel (boundary->getFeatherFunction ()));
	m_editFeatherDistance.LinkSmartSliderCtrl (&m_sliderFeatherDistance);
	m_editFeatherDistance.SetParams (0.0f, 1.0f, 20, "%1.5f");
	IGNORE_RETURN (m_editFeatherDistance.SetValue (boundary->getFeatherDistance ()));
	m_isWaterTable = boundary->isLocalWaterTable ();
	m_isLocalGlobalWaterTable = boundary->isLocalGlobalWaterTable ();
	m_waterHeight = boundary->getLocalWaterTableHeight ();
	m_waterShader.SetWindowText (boundary->getLocalWaterTableShaderTemplateName ());
	m_waterShaderSize = boundary->getLocalWaterTableShaderSize ();
	IGNORE_RETURN (m_waterType.SetCurSel (boundary->getWaterType ()));


	IGNORE_RETURN (m_waterHeight.EnableWindow (m_isWaterTable ? TRUE : FALSE));
	IGNORE_RETURN (m_waterShader.EnableWindow (m_isWaterTable ? TRUE : FALSE));
	IGNORE_RETURN (m_waterShaderSize.EnableWindow (m_isWaterTable ? TRUE : FALSE));
	IGNORE_RETURN (m_browseButton.EnableWindow (m_isWaterTable ? TRUE : FALSE));
	IGNORE_RETURN (m_localGlobalWaterTable.EnableWindow (m_isWaterTable ? TRUE : FALSE));
	IGNORE_RETURN (m_waterType.EnableWindow (m_isWaterTable ? TRUE : FALSE));

	IGNORE_RETURN (UpdateData (false));
}

//-------------------------------------------------------------------

void FormBoundaryRect::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormBoundaryRect::HasChanged () const
{
	const Rectangle2d rect = boundary->getRectangle ();

	CString waterShader;
	m_waterShader.GetWindowText (waterShader);

	return 
		(m_x0                                   != rect.x0) ||
		(m_z0                                   != rect.y0) ||
		(m_x1                                   != rect.x1) ||
		(m_z1                                   != rect.y1) ||
		(m_featherFunction.GetCurSel ()         != boundary->getFeatherFunction ()) ||
		(m_editFeatherDistance.GetValueFloat () != boundary->getFeatherDistance ()) ||
		((m_isWaterTable == TRUE)       != boundary->isLocalWaterTable ()) ||
		((m_isLocalGlobalWaterTable == TRUE)    != boundary->isLocalGlobalWaterTable ()) ||
		(m_waterHeight                  != boundary->getLocalWaterTableHeight ()) ||
		(waterShader                    != (boundary->getLocalWaterTableShaderTemplateName () ? boundary->getLocalWaterTableShaderTemplateName () : "")) ||
		(m_waterType.GetCurSel () != boundary->getWaterType()) ||
		(m_waterShaderSize              != boundary->getLocalWaterTableShaderSize ());
}

//-------------------------------------------------------------------

void FormBoundaryRect::OnSelchangeWaterType() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormBoundaryRect::ApplyChanges ()
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (boundary);

		Rectangle2d newRect;
		newRect.x0 = m_x0;
		newRect.y0 = m_z0;
		newRect.x1 = m_x1;
		newRect.y1 = m_z1;

		CString waterShader;
		m_waterShader.GetWindowText (waterShader);

		boundary->setFeatherFunction (static_cast<TerrainGeneratorFeatherFunction> (m_featherFunction.GetCurSel ()));
		boundary->setFeatherDistance (m_editFeatherDistance.GetValueFloat ());
		boundary->setRectangle (newRect);
		boundary->setLocalWaterTable (m_isWaterTable == TRUE);
		boundary->setLocalGlobalWaterTable (m_isLocalGlobalWaterTable == TRUE);
		boundary->setLocalWaterTableHeight (m_waterHeight);
		boundary->setLocalWaterTableShaderTemplateName (waterShader);
		boundary->setLocalWaterTableShaderSize (m_waterShaderSize);
		boundary->setWaterType (static_cast<TerrainGeneratorWaterType> (m_waterType.GetCurSel ()));

		IGNORE_RETURN (m_waterHeight.EnableWindow (m_isWaterTable ? TRUE : FALSE));
		IGNORE_RETURN (m_waterShader.EnableWindow (m_isWaterTable ? TRUE : FALSE));
		IGNORE_RETURN (m_waterShaderSize.EnableWindow (m_isWaterTable ? TRUE : FALSE));
		IGNORE_RETURN (m_browseButton.EnableWindow (m_isWaterTable ? TRUE : FALSE));
		IGNORE_RETURN (m_localGlobalWaterTable.EnableWindow (m_isWaterTable ? TRUE : FALSE));
		IGNORE_RETURN (m_waterType.EnableWindow (m_isWaterTable ? TRUE : FALSE));

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormBoundaryRect::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	UNREF (pSender);
	UNREF (lHint);
	UNREF (pHint);

	if (pSender)
	{
		update ();
	}
}

//-------------------------------------------------------------------

void FormBoundaryRect::OnSelchangeFeatherFunction() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormBoundaryRect::OnButtonShaderBrowse() 
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

void FormBoundaryRect::OnCheckIswatertable() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormBoundaryRect::OnCheckLocalglobalwatertable() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

