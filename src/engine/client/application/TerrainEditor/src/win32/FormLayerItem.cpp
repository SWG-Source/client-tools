//
// FormLayerItem.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormLayerItem.h"

//-------------------------------------------------------------------

FormLayerItem::FormLayerItemViewData::FormLayerItemViewData (void) :
	ViewData (),
	item (0)
{
}

//-------------------------------------------------------------------

FormLayerItem::FormLayerItemViewData::~FormLayerItemViewData (void)
{
	item = 0;
}

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormLayerItem, PropertyView)

//-------------------------------------------------------------------

FormLayerItem::FormLayerItem() : 
	PropertyView(IDD)
{
	//{{AFX_DATA_INIT(FormLayerItem)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------

FormLayerItem::FormLayerItem(UINT nIDTemplate) : 
	PropertyView(nIDTemplate)
{
}

//-------------------------------------------------------------------

FormLayerItem::~FormLayerItem()
{
}

//-------------------------------------------------------------------

void FormLayerItem::DoDataExchange(CDataExchange* pDX)
{
	PropertyView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormLayerItem)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FormLayerItem, PropertyView)
	//{{AFX_MSG_MAP(FormLayerItem)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormLayerItem::AssertValid() const
{
	PropertyView::AssertValid();
}

void FormLayerItem::Dump(CDumpContext& dc) const
{
	PropertyView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormLayerItem::Initialize (ViewData* vd)
{
	FATAL (true, ("FormLayerItem::Initialize called -- derived classes should override this function"));
	UNREF (vd);
}

//-------------------------------------------------------------------

bool FormLayerItem::HasChanged () const
{
	FATAL (true, ("FormLayerItem::HasChanged called -- derived classes should override this function"));
	return false;
}

//-------------------------------------------------------------------

void FormLayerItem::ApplyChanges ()
{
	FATAL (true, ("FormLayerItem::ApplyChanges called -- derived classes should override this function"));
}

//-------------------------------------------------------------------


