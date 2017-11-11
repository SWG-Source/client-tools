// PreviewDialog.cpp : implementation file
//

#include "stdafx.h"
#include "PreviewDialog.h"

#include "UIFontBuilder.h"
#include "FontGlyphCode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPreviewDialog dialog

using Unicode::unicode_char_t;

CPreviewDialog::CPreviewDialog(CWnd* pParent, const std::set<unicode_char_t> & idSet, CFont * font, int fontSizePixels)
	: CDialog(CPreviewDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPreviewDialog)
	m_textInfoValue = _T("");
	m_textInfoExcludedValue = _T("");
	//}}AFX_DATA_INIT

	m_idSet = idSet;

	TCHAR buf [1024];

	_snwprintf (buf, 1023, _T("%d characters in range.\n"), m_idSet.size ());

	m_textInfoExcludedValue = _T ("0 characters excluded.\n");
	m_textInfoValue = buf;

	m_font = font;
	m_fontSizePixels = fontSizePixels;
}


void CPreviewDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPreviewDialog)
	DDX_Control(pDX, IDC_LIST_PREVIEW, m_listPreview);
	DDX_Text(pDX, IDC_TEXT_INFO, m_textInfoValue);
	DDX_Text(pDX, IDC_TEXT_INFO_EXCLUDED, m_textInfoExcludedValue);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPreviewDialog, CDialog)
	//{{AFX_MSG_MAP(CPreviewDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PREVIEW, OnItemchangedListPreview)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPreviewDialog message handlers

BOOL CPreviewDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_listPreview.SetFont (m_font);

	m_listPreview.InsertColumn (0, _T("Code"), LVCFMT_LEFT, 100);
	m_listPreview.InsertColumn (1, _T("Sample"), LVCFMT_LEFT, 100);
	m_listPreview.InsertColumn (2, _T("A"), LVCFMT_LEFT, 100);
	m_listPreview.InsertColumn (3, _T("B"), LVCFMT_LEFT, 100);
	m_listPreview.InsertColumn (4, _T("C"), LVCFMT_LEFT, 100);
	m_listPreview.InsertColumn (5, _T("Glyph"), LVCFMT_LEFT, 100);

	TCHAR buf[2] = {0,0};

	TCHAR numbuf[5] = {0,0,0,0,0};

	CDC     *myDC = GetDC();
	HDC     ScreenDC;		
	ScreenDC     = (HDC)(*myDC);

	HFONT   TheFont;
	LOGFONT lfont;		

	m_font->GetLogFont (&lfont);

	TheFont = CreateFont(
		m_fontSizePixels,
		0, 
		0, 
		0, 
		lfont.lfWeight,
		0,
		0,
		0,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, 	
		CLIP_DEFAULT_PRECIS,
		PROOF_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		lfont.lfFaceName );
	
	myDC->SelectObject(TheFont);

	size_t i = 0;
	
	for (std::set<unicode_char_t>::const_iterator iter = m_idSet.begin (); iter != m_idSet.end (); ++iter, ++i)
	{
		buf[0] = *iter;

		_snwprintf (numbuf, 4, _T ("%04x"), *iter);
		m_listPreview.InsertItem (i, numbuf);
		m_listPreview.SetItemData (i, *iter);

		m_listPreview.SetItemText (i, 1, buf);
		
		ABCFLOAT abcf;
		USHORT const index = GetTTUnicodeGlyphIndex(ScreenDC, *iter);
		if(index == 0)
			abcf.abcfA = abcf.abcfB = abcf.abcfC = -1;
		else
		{
			if(!myDC->GetCharABCWidths(*iter, *iter, &abcf))
			{
				int err = GetLastError();
				TCHAR t[512];
				_snwprintf(t, 512, _T("%x\n"), err);
				OutputDebugString(t);
			}
		}
		
		_snwprintf (numbuf, 4, _T ("%4.1f"), abcf.abcfA);
		m_listPreview.SetItemText(i, 2, numbuf);
		_snwprintf (numbuf, 4, _T ("%4.1f"), abcf.abcfB);
		m_listPreview.SetItemText(i, 3, numbuf);
		_snwprintf (numbuf, 4, _T ("%4.1f"), abcf.abcfC);
		m_listPreview.SetItemText(i, 4, numbuf);
		_snwprintf (numbuf, 4, _T ("%4d"), index);
		m_listPreview.SetItemText(i, 5, numbuf);
	}

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//-----------------------------------------------------------------

const CPreviewDialog::ExclusionVector_t & CPreviewDialog::getExclusionVector () const
{
	return m_exclusionVector;

}

void CPreviewDialog::OnOK() 
{
	// TODO: Add extra validation here
	POSITION pos = m_listPreview.GetFirstSelectedItemPosition ();

	while (pos)
	{
		int nItem = m_listPreview.GetNextSelectedItem (pos);
		m_exclusionVector.push_back (static_cast<Unicode::unicode_char_t>(m_listPreview.GetItemData (nItem)));
	}

	CDialog::OnOK();
}

void CPreviewDialog::OnItemchangedListPreview(NMHDR*, LRESULT* pResult) 
{
	
	//NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// TODO: this gets called every type the selection changes, so we should really
	// try to do the text field update less often!

	size_t selcount = m_listPreview.GetSelectedCount ();

	TCHAR buf[128];

	_snwprintf (buf, 127, _T("%d characters excluded."), selcount);

	m_textInfoExcludedValue = buf;

	UpdateData (false);

	*pResult = 0;
}
