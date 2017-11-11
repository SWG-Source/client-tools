//
// FormTemplate.cpp
// asommers
// 
// copyright 2000, sony online entertainment

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormTemplate.h"

#include "sharedTerrain/Affector.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormTemplate, FormLayerItem)

//-------------------------------------------------------------------

FormTemplate::FormTemplate() : 
	FormLayerItem(FormTemplate::IDD),
	affector (0)
{
	//{{AFX_DATA_INIT(FormTemplate)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------
	
FormTemplate::~FormTemplate()
{
	affector = 0;
}

//-------------------------------------------------------------------

void FormTemplate::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormTemplate)
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormTemplate, PropertyView)
	//{{AFX_MSG_MAP(FormTemplate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormTemplate::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormTemplate::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormTemplate::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (affector);

	IGNORE_RETURN (UpdateData (FALSE));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormTemplate::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

//	affector = dynamic_cast<Affector*>(flivd->item->layerItem);
//	NOT_NULL (affector);
}

//-------------------------------------------------------------------

bool FormTemplate::HasChanged (void) const
{
	return true;
}

//-------------------------------------------------------------------

void FormTemplate::ApplyChanges (void)
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (TRUE));

	if (HasChanged ())
	{
//		NOT_NULL (affector);

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

