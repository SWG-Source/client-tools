//
// IffDataView.cpp
// asommers 6-12-99
//
// copyright 1999, bootprint entertainment
//

//-------------------------------------------------------------------

#include "FirstViewIff.h"
#include "viewiff.h"

#include "ChildFrm.h"
#include "IffDoc.h"
#include "IffDataView.h"
#include "IffTreeView.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(IffDataView, CEditView)

BEGIN_MESSAGE_MAP(IffDataView, CEditView)
	//{{AFX_MSG_MAP(IffDataView)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CEditView::OnFilePrintPreview)
END_MESSAGE_MAP()

//-------------------------------------------------------------------

IffDataView::IffDataView()
{
	displayMode = DM_dword;
	font        = 0;
}

IffDataView::~IffDataView()
{
	delete font;
	font = 0;
}

//-------------------------------------------------------------------

BOOL IffDataView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= ES_MULTILINE;
	cs.style |= WS_VSCROLL;

	return CEditView::PreCreateWindow(cs);
}

//-------------------------------------------------------------------

void IffDataView::OnDraw(CDC* pDC)
{
	IffDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	UNREF (pDC);
}

//-------------------------------------------------------------------

void IffDataView::OnInitialUpdate()
{
	CEditView::OnInitialUpdate();

	//-- make sure the data view is read-only
	GetEditCtrl ().SetReadOnly (true);

	//-- set the font
	CFont* font = new CFont;
	font->CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, NULL);
	SetFont (font);
}

//-------------------------------------------------------------------

BOOL IffDataView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void IffDataView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void IffDataView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void IffDataView::AssertValid() const
{
	CEditView::AssertValid();
}

void IffDataView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

IffDoc* IffDataView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(IffDoc)));
	return (IffDoc*)m_pDocument;
}
#endif //_DEBUG

//-------------------------------------------------------------------

void IffDataView::updateOutput (void)
{
	static char hexArray [] = "0123456789ABCDEF";

	IffDoc* pDoc = GetDocument ();

	CTreeCtrl& treeCtrl = static_cast<CChildFrame*> (GetParentFrame ())->getTreeView ()->GetTreeCtrl ();
	HTREEITEM  item     = treeCtrl.GetSelectedItem ();

	if (!item)
		return;

	const IffDoc::IffNode* node = pDoc->findNode (item);

	const char* const data   = reinterpret_cast<const char* const> (node->data);
	int               length = node->length;

	CString m_output = "Selected item is a FORM and has no data";

	if (data)
	{
		m_output = "";

		switch (displayMode)
		{
		default:
			MessageBox ("unknown display mode");
			break;

		case DM_text:
			{
				int i;
				for (i = 0; i < length; i++)
					if (static_cast<int> (data [i]) >= 32 && static_cast<int> (data [i]) <= 122)
						m_output += data [i];
					else
						m_output += ".";
			}
			break;

		case DM_byte:
			{
				char digit [3];

				m_output = "";

				int i;
				for (i = 0; i < length; i++)
				{
					digit [0] = hexArray [(data [i] >> 4) & 0x0F];
					digit [1] = hexArray [(data [i]) & 0x0F];
					digit [2] = 0;

					m_output += digit;
					m_output += " ";
				}
			}
			break;

		case DM_word:
			{
				char digit [5];

				m_output = "";

				int i;
				for (i = 0; i < length; i++)
				{
					memset (digit, 0, 5);

					digit [2] = hexArray [(data [i] >> 4) & 0x0F];
					digit [3] = hexArray [(data [i]) & 0x0F];
					i++;

					if (i < length)
					{
						digit [0] = hexArray [(data [i] >> 4) & 0x0F];
						digit [1] = hexArray [(data [i]) & 0x0F];
					}

					m_output += digit;
					m_output += " ";
				}
			}
			break;

		case DM_dword:
			{
				char digit [9];

				m_output = "";

				int i;
				for (i = 0; i < length; i++)
				{
					memset (digit, 0, 9);

					digit [6] = hexArray [(data [i] >> 4) & 0x0F];
					digit [7] = hexArray [(data [i]) & 0x0F];

					i++;
					if (i < length)
					{
						digit [4] = hexArray [(data [i] >> 4) & 0x0F];
						digit [5] = hexArray [(data [i]) & 0x0F];
					}

					i++;
					if (i < length)
					{
						digit [2] = hexArray [(data [i] >> 4) & 0x0F];
						digit [3] = hexArray [(data [i]) & 0x0F];
					}

					i++;
					if (i < length)
					{
						digit [0] = hexArray [(data [i] >> 4) & 0x0F];
						digit [1] = hexArray [(data [i]) & 0x0F];
					}

					m_output += digit;
					m_output += " ";
				}
			}
			break;
		}
	}

	GetEditCtrl ().SetWindowText (m_output);
}

void IffDataView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CEditView::OnUpdate (pSender, lHint, pHint);
	
	// TODO: Add your specialized code here and/or call the base class
	updateOutput ();
}

void IffDataView::OnDestroy() 
{
	CEditView::OnDestroy();
}
