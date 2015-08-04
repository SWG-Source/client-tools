//
// FormRadial.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormRadial.h"

#include "sharedTerrain/RadialGroup.h"

//-------------------------------------------------------------------

FormRadial::FormRadialViewData::FormRadialViewData () : 
	ViewData (),
	childName (),
	familyId (0),
	radialGroup (0)
{
}

//-------------------------------------------------------------------

FormRadial::FormRadialViewData::~FormRadialViewData ()
{
	radialGroup = 0;
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormRadial, PropertyView)

//-------------------------------------------------------------------

FormRadial::FormRadial() :
	PropertyView(FormRadial::IDD),
	data (),

	//-- widgets
	m_familyColorWindow (),
	m_name (),
	m_weight (false),
	m_familyName (),
	m_distance (false),
	m_minHeight (false),
	m_maxHeight (false),
	m_minWidth (false),
	m_maxWidth (false),
	m_shouldSway (FALSE),
	m_displacement (false),
	m_period (false),
	m_createPlus (FALSE),
	m_maintainAspectRatio (FALSE),
	m_alignToTerrain (FALSE)
{
	//{{AFX_DATA_INIT(FormRadial)
	m_name = _T("");
	m_familyName = _T("");
	m_shouldSway = FALSE;
	m_createPlus = FALSE;
	m_maintainAspectRatio = FALSE;
	m_alignToTerrain = FALSE;
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------

FormRadial::~FormRadial()
{
}

//-------------------------------------------------------------------

void FormRadial::DoDataExchange(CDataExchange* pDX)
{
	PropertyView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormRadial)
	DDX_Control(pDX, IDC_EDIT_DISTANCE, m_distance);
	DDX_Control(pDX, IDC_EDIT_MINHEIGHT, m_minHeight);
	DDX_Control(pDX, IDC_EDIT_MAXHEIGHT, m_maxHeight);
	DDX_Control(pDX, IDC_EDIT_MINWIDTH, m_minWidth);
	DDX_Control(pDX, IDC_EDIT_MAXWIDTH, m_maxWidth);
	DDX_Control(pDX, IDC_WEIGHT, m_weight);
	DDX_Control(pDX, IDC_EDIT_DISPLACEMENT, m_displacement);
	DDX_Control(pDX, IDC_EDIT_FREQUENCY, m_period);
	DDX_Control(pDX, IDC_STATIC_FAMILY_COLOR, m_familyColorWindow);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Text(pDX, IDC_EDIT_FAMILY_NAME, m_familyName);
	DDX_Check(pDX, IDC_CHECK_SWAY, m_shouldSway);
	DDX_Check(pDX, IDC_CREATE_PLUS, m_createPlus);
	DDX_Check(pDX, IDC_MAINTAIN_ASPECT_RATIO, m_maintainAspectRatio);
	DDX_Check(pDX, IDC_ALIGN_TO_TERRAIN, m_alignToTerrain);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormRadial, PropertyView)
	//{{AFX_MSG_MAP(FormRadial)
	ON_BN_CLICKED(IDC_CHECK_SWAY, OnCheckSway)
	ON_BN_CLICKED(IDC_ALIGN_TO_TERRAIN, OnAlignToTerrain)
	ON_BN_CLICKED(IDC_CREATE_PLUS, OnCreatePlus)
	ON_BN_CLICKED(IDC_MAINTAIN_ASPECT_RATIO, OnMaintainAspectRatio)
	ON_EN_CHANGE(IDC_EDIT_DISPLACEMENT, OnChangeEditDisplacement)
	ON_EN_CHANGE(IDC_EDIT_DISTANCE, OnChangeEditDistance)
	ON_EN_CHANGE(IDC_EDIT_FREQUENCY, OnChangeEditFrequency)
	ON_EN_CHANGE(IDC_EDIT_MINHEIGHT, OnChangeEditMinHeight)
	ON_EN_CHANGE(IDC_EDIT_MAXHEIGHT, OnChangeEditMaxHeight)
	ON_EN_CHANGE(IDC_EDIT_MINWIDTH, OnChangeEditMinWidth)
	ON_EN_CHANGE(IDC_EDIT_MAXWIDTH, OnChangeEditMaxWidth)
	ON_EN_CHANGE(IDC_WEIGHT, OnChangeWeight)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormRadial::AssertValid() const
{
	PropertyView::AssertValid();
}

void FormRadial::Dump(CDumpContext& dc) const
{
	PropertyView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormRadial::OnDraw(CDC* pDC) 
{
	// TODO: Add your specialized code here and/or call the base class
	PropertyView::OnDraw (pDC);

	// TODO: Add your specialized code here and/or call the base class
	PackedRgb color = data.radialGroup->getFamilyColor (data.familyId);
	
	CRect rect;
	m_familyColorWindow.GetWindowRect (&rect);
	ScreenToClient (&rect);

	CBrush brush;
	IGNORE_RETURN (brush.CreateSolidBrush (RGB (color.r, color.g, color.b)));
	IGNORE_RETURN (pDC->SelectObject (&brush));
	IGNORE_RETURN (pDC->Rectangle (&rect));
}

//-------------------------------------------------------------------

void FormRadial::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormRadialViewData* frvd = dynamic_cast<FormRadialViewData*> (vd);
	NOT_NULL (frvd);

	data = *frvd;
}

//-------------------------------------------------------------------------

void FormRadial::OnInitialUpdate() 
{
	PropertyView::OnInitialUpdate();

	m_name       = data.childName;
	m_familyName = data.radialGroup->getFamilyName (data.familyId);

	const RadialGroup::FamilyChildData fcd = data.radialGroup->getFamilyChild (data.familyId, data.childName);

	m_minWidth       = fcd.minWidth;
	m_maxWidth       = fcd.maxWidth;
	m_minHeight      = fcd.minHeight;
	m_maxHeight      = fcd.maxHeight;
	m_weight         = fcd.weight;
	m_distance       = fcd.distance;
	m_shouldSway     = fcd.shouldSway;
	m_displacement   = fcd.displacement;
	m_period         = fcd.period;
	m_alignToTerrain = fcd.alignToTerrain;
	m_createPlus     = fcd.createPlus;
	m_maintainAspectRatio = fcd.maintainAspectRatio;

	IGNORE_RETURN (UpdateData (false));	

	IGNORE_RETURN (m_displacement.EnableWindow (m_shouldSway));
	IGNORE_RETURN (m_period.EnableWindow (m_shouldSway));
	IGNORE_RETURN (m_maxHeight.EnableWindow (!m_maintainAspectRatio));

	m_initialized = true;
}

//-------------------------------------------------------------------

bool FormRadial::HasChanged () const
{
	const RadialGroup::FamilyChildData fcd = data.radialGroup->getFamilyChild (data.familyId, data.childName);

	return

		(m_minWidth                    != fcd.minWidth) ||
		(m_maxWidth                    != fcd.maxWidth) ||
		(m_minHeight                   != fcd.minHeight) ||
		(m_maxHeight                   != fcd.maxHeight) ||
		(m_weight                   != fcd.weight) ||
		(m_distance                 != fcd.distance) ||
		((m_shouldSway == TRUE)     != fcd.shouldSway) ||
		(m_displacement             != fcd.displacement) ||
		(m_period                   != fcd.period) ||
		((m_alignToTerrain == TRUE) != fcd.alignToTerrain) ||
		((m_maintainAspectRatio == TRUE) != fcd.maintainAspectRatio) ||
		((m_createPlus == TRUE)     != fcd.createPlus);
}

//-------------------------------------------------------------------

void FormRadial::ApplyChanges ()
{
	if (!m_initialized)
		return;


	IGNORE_RETURN (UpdateData (true));	

	if (HasChanged ())
	{
		NOT_NULL (data.radialGroup);

		RadialGroup::FamilyChildData fcd;
		fcd.familyId           = data.familyId;
		fcd.weight             = m_weight;
		fcd.shaderTemplateName = data.childName;
		fcd.distance           = m_distance;
		fcd.minWidth           = m_minWidth;
		fcd.maxWidth           = m_maxWidth;
		fcd.minHeight          = m_minHeight;
		fcd.maxHeight          = m_maxHeight;
		fcd.shouldSway         = m_shouldSway == TRUE;
		fcd.period             = m_period;
		fcd.displacement       = m_displacement;
		fcd.alignToTerrain     = m_alignToTerrain == TRUE;
		fcd.createPlus         = m_createPlus == TRUE;
		fcd.maintainAspectRatio = m_maintainAspectRatio == TRUE;

		data.radialGroup->setFamilyChild (data.familyId, fcd);

		IGNORE_RETURN (m_displacement.EnableWindow (m_shouldSway));
		IGNORE_RETURN (m_period.EnableWindow (m_shouldSway));
		IGNORE_RETURN (m_maxHeight.EnableWindow (!m_maintainAspectRatio));
		if(m_maintainAspectRatio)
		{
			float ratio = 1.0f;
			if(m_maxWidth != 0.0f)
			{
				ratio = m_minWidth/m_maxWidth;
				if(ratio == 0.0f)
					ratio = 1.0f;
			}
			m_maxHeight = m_minHeight/ratio;
		}

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormRadial::OnCheckSway() 
{
	ApplyChanges ();	
}

//-------------------------------------------------------------------

void FormRadial::OnAlignToTerrain() 
{
	ApplyChanges ();	
}

//-------------------------------------------------------------------

void FormRadial::OnCreatePlus() 
{
	ApplyChanges ();	
}

//-------------------------------------------------------------------

void FormRadial::OnMaintainAspectRatio() 
{
	ApplyChanges ();	
}

//-------------------------------------------------------------------

void FormRadial::OnChangeEditDisplacement() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormRadial::OnChangeEditDistance() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormRadial::OnChangeEditFrequency() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormRadial::OnChangeEditMinHeight() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormRadial::OnChangeEditMaxHeight() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormRadial::OnChangeEditMinWidth() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormRadial::OnChangeEditMaxWidth() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormRadial::OnChangeWeight() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

