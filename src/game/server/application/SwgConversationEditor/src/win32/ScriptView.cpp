// ======================================================================
//
// ScriptView.cpp
// asommers 2003-09-23
//
// copyright2003, sony online entertainment
// 
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "ScriptView.h"

#include "ScriptGroup.h"
#include "SwgConversationEditor.h"
#include "SwgConversationEditorDoc.h"

#pragma warning (disable: 4355)

// ======================================================================

IMPLEMENT_DYNCREATE(ScriptView, CCrystalEditView)

// ======================================================================

ScriptView::ScriptView() :
	m_familyId (0),
	m_type (T_nothing),
	m_textBuffer (this)
{
	//	Initialize LOGFONT structure
	memset(&m_lf, 0, sizeof(m_lf));
	m_lf.lfWeight = FW_NORMAL;
	m_lf.lfCharSet = ANSI_CHARSET;
	m_lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	m_lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	m_lf.lfQuality = DEFAULT_QUALITY;
	m_lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	strcpy(m_lf.lfFaceName, "Courier");

	m_textBuffer.InitNew (CRLF_STYLE_UNIX);
}

// ----------------------------------------------------------------------

ScriptView::~ScriptView()
{
	m_textBuffer.FreeAll ();
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(ScriptView, CCrystalEditView)
	//{{AFX_MSG_MAP(ScriptView)
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

void ScriptView::OnDraw(CDC * const pDC)
{
	CCrystalEditView::OnDraw (pDC);
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void ScriptView::AssertValid() const
{
	CCrystalEditView::AssertValid();
}

void ScriptView::Dump(CDumpContext& dc) const
{
	CCrystalEditView::Dump(dc);
}
#endif //_DEBUG

// ======================================================================

CCrystalTextBuffer * ScriptView::LocateTextBuffer ()
{
	return &m_textBuffer;
}

// ----------------------------------------------------------------------

ScriptGroup * ScriptView::getScriptGroup ()
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	switch (m_type)
	{
	case T_nothing:
	default:
		break;

	case T_condition:
		return document->getConditionGroup ();

	case T_action:
		return document->getActionGroup ();

	case T_tokenTO:
		return document->getTokenTOGroup ();

	case T_tokenDI:
		return document->getTokenDIGroup ();

	case T_tokenDF:
		return document->getTokenDFGroup ();
	}

	return 0;
}

// ----------------------------------------------------------------------

void ScriptView::editCondition (int const conditionFamilyId)
{
	if (m_type != T_condition || m_familyId != conditionFamilyId)
		OnChange ();

	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	m_type = T_condition;
	m_familyId = conditionFamilyId;

	CString const text = document->getConditionGroup ()->getFamilyText (m_familyId);
	m_textBuffer.SetText (text);
	m_textBuffer.SetReadOnly (m_familyId == 0);

	Invalidate ();
}

// ----------------------------------------------------------------------

void ScriptView::editAction (int const actionFamilyId)
{
	if (m_type != T_action || m_familyId != actionFamilyId)
		OnChange ();

	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	m_type = T_action;
	m_familyId = actionFamilyId;

	CString const text = document->getActionGroup ()->getFamilyText (m_familyId);
	m_textBuffer.SetText (text);
	m_textBuffer.SetReadOnly (m_familyId == 0);

	Invalidate ();
}

// ----------------------------------------------------------------------

void ScriptView::editTokenTO (int const tokenTOFamilyId)
{
	if (m_type != T_tokenTO || m_familyId != tokenTOFamilyId)
		OnChange ();

	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	m_type = T_tokenTO;
	m_familyId = tokenTOFamilyId;

	CString const text = document->getTokenTOGroup ()->getFamilyText (m_familyId);
	m_textBuffer.SetText (text);
	m_textBuffer.SetReadOnly (m_familyId == 0);

	Invalidate ();
}

// ----------------------------------------------------------------------

void ScriptView::editTokenDI (int const tokenDIFamilyId)
{
	if (m_type != T_tokenDI || m_familyId != tokenDIFamilyId)
		OnChange ();

	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	m_type = T_tokenDI;
	m_familyId = tokenDIFamilyId;

	CString const text = document->getTokenDIGroup ()->getFamilyText (m_familyId);
	m_textBuffer.SetText (text);
	m_textBuffer.SetReadOnly (m_familyId == 0);

	Invalidate ();
}

// ----------------------------------------------------------------------

void ScriptView::editTokenDF (int const tokenDFFamilyId)
{
	if (m_type != T_tokenDF || m_familyId != tokenDFFamilyId)
		OnChange ();

	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	m_type = T_tokenDF;
	m_familyId = tokenDFFamilyId;

	CString const text = document->getTokenDFGroup ()->getFamilyText (m_familyId);
	m_textBuffer.SetText (text);
	m_textBuffer.SetReadOnly (m_familyId == 0);

	Invalidate ();
}

// ----------------------------------------------------------------------

void ScriptView::editTrigger ()
{
	if (m_type != T_trigger || m_familyId != 0)
		OnChange ();

	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	m_type = T_trigger;
	m_familyId = 0;

	CString const text = document->getConversation ()->getTriggerText ().c_str ();
	m_textBuffer.SetText (text);
	m_textBuffer.SetReadOnly (false);

	Invalidate ();
}

// ----------------------------------------------------------------------

void ScriptView::editNothing ()
{
	if (m_type != T_nothing || m_familyId != 0)
		OnChange ();

	m_type = T_nothing;
	m_familyId = 0;

	m_textBuffer.SetText ("");
	m_textBuffer.SetReadOnly (true);

	Invalidate ();
}

// ----------------------------------------------------------------------

void ScriptView::OnChange() 
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	CString const buffer = m_textBuffer.GetText ();

	switch (m_type)
	{
	case T_nothing:
	default:
		break;

	case T_condition:
	case T_action:
	case T_tokenTO:
	case T_tokenDF:
	case T_tokenDI:
		{
			ScriptGroup * const scriptGroup = getScriptGroup ();
			if (scriptGroup)
				scriptGroup->setFamilyText (m_familyId, buffer);
		}
		break;

	case T_trigger:
		{
			document->getConversation ()->setTriggerText (std::string (buffer));
		}
		break;
	}

	document->SetModifiedFlag ();
}

// ----------------------------------------------------------------------

void ScriptView::OnInitialUpdate() 
{
	CCrystalEditView::OnInitialUpdate();

	SetFont (m_lf);
}

// ----------------------------------------------------------------------

void ScriptView::OnUpdate(CView * const pSender, LPARAM const lHint, CObject* /*pHint*/) 
{
	if (this != pSender)
	{
		switch (lHint)
		{
		case SwgConversationEditorDoc::H_savingDocument:
		case SwgConversationEditorDoc::H_scriptChanged:
			fixFunctionName ();
			break;

		case SwgConversationEditorDoc::H_triggerChanged:
			editTrigger ();
			break;

		default:
			break;
		}
	}
}

// ----------------------------------------------------------------------

void ScriptView::fixFunctionName ()
{
	CString const oldText (m_textBuffer.GetText ());
	CString text (oldText);
	bool changed = false;

	switch (m_type)
	{
	case T_condition:
		{
			ScriptGroup * const scriptGroup = getScriptGroup ();
			if (scriptGroup)
			{
				text = SwgConversationEditorDoc::fixConditionFunctionName (text, "", scriptGroup, m_familyId);
				changed = true;
			}
		}
		break;

	case T_action:
		{
			ScriptGroup * const scriptGroup = getScriptGroup ();
			if (scriptGroup)
			{
				text = SwgConversationEditorDoc::fixActionFunctionName (text, "", scriptGroup, m_familyId);
				changed = true;
			}
		}
		break;

	case T_tokenTO:
		{
			ScriptGroup * const scriptGroup = getScriptGroup ();
			if (scriptGroup)
			{
				text = SwgConversationEditorDoc::fixTokenTOFunctionName (text, "", scriptGroup, m_familyId);
				changed = true;
			}
		}
		break;

	case T_tokenDI:
		{
			ScriptGroup * const scriptGroup = getScriptGroup ();
			if (scriptGroup)
			{
				text = SwgConversationEditorDoc::fixTokenDIFunctionName (text, "", scriptGroup, m_familyId);
				changed = true;
			}
		}
		break;

	case T_tokenDF:
		{
			ScriptGroup * const scriptGroup = getScriptGroup ();
			if (scriptGroup)
			{
				text = SwgConversationEditorDoc::fixTokenDFFunctionName (text, "", scriptGroup, m_familyId);
				changed = true;
			}
		}
		break;
	}

	if (changed && text != oldText)
		m_textBuffer.SetText (text);

	OnChange ();
}

// ----------------------------------------------------------------------

void ScriptView::OnKillFocus(CWnd* pNewWnd) 
{
	CCrystalEditView::OnKillFocus(pNewWnd);

	fixFunctionName ();
}

// ----------------------------------------------------------------------

BOOL ScriptView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= (WS_HSCROLL | WS_VSCROLL);
	
	return CCrystalEditView::PreCreateWindow(cs);
}

// ======================================================================

//	C++ keywords (MSVC5.0 + POET5.0)
static LPTSTR s_apszCppKeywordList[] =
{
	_T("string"),
	_T("obj_id"),
	_T("self"),
	_T("boolean"),
	_T("false"),
	_T("true"),
	_T("case"),
	_T("try"),
	_T("catch"),
	_T("new"),
	_T("float"),
	_T("char"),
	_T("for"),
	_T("private"),
	_T("class"),
	_T("protected"),
	_T("const"),
	_T("public"),
	_T("union"),
	_T("if"),
	_T("unsigned"),
	_T("continue"),
	_T("return"),
	_T("default"),
	_T("int"),
	_T("short"),
	_T("signed"),
	_T("void"),
	_T("static"),
	_T("do"),
	_T("double"),
	_T("while"),
	_T("long"),
	_T("else"),
	_T("switch"),
	_T("string_id"),
	NULL
};

static BOOL IsCppKeyword(LPCTSTR pszChars, int nLength)
{
	for (int L = 0; s_apszCppKeywordList[L] != NULL; L ++)
	{
		if (strncmp(s_apszCppKeywordList[L], pszChars, nLength) == 0
				&& s_apszCppKeywordList[L][nLength] == 0)
			return TRUE;
	}
	return FALSE;
}

static BOOL IsCppNumber(LPCTSTR pszChars, int nLength)
{
	if (nLength > 2 && pszChars[0] == '0' && pszChars[1] == 'x')
	{
		for (int I = 2; I < nLength; I++)
		{
			if (isdigit(pszChars[I]) || (pszChars[I] >= 'A' && pszChars[I] <= 'F') ||
										(pszChars[I] >= 'a' && pszChars[I] <= 'f'))
				continue;
			return FALSE;
		}
		return TRUE;
	}
	if (! isdigit(pszChars[0]))
		return FALSE;
	for (int I = 1; I < nLength; I++)
	{
		if (! isdigit(pszChars[I]) && pszChars[I] != '+' &&
			pszChars[I] != '-' && pszChars[I] != '.' && pszChars[I] != 'e' &&
			pszChars[I] != 'E')
			return FALSE;
	}
	return TRUE;
}

#define DEFINE_BLOCK(pos, colorindex)	\
	ASSERT((pos) >= 0 && (pos) <= nLength);\
	if (pBuf != NULL)\
	{\
		if (nActualItems == 0 || pBuf[nActualItems - 1].m_nCharPos <= (pos)){\
		pBuf[nActualItems].m_nCharPos = (pos);\
		pBuf[nActualItems].m_nColorIndex = (colorindex);\
		nActualItems ++;}\
	}

#define COOKIE_COMMENT			0x0001
#define COOKIE_PREPROCESSOR		0x0002
#define COOKIE_EXT_COMMENT		0x0004
#define COOKIE_STRING			0x0008
#define COOKIE_CHAR				0x0010

DWORD ScriptView::ParseLine(DWORD dwCookie, int nLineIndex, TEXTBLOCK *pBuf, int &nActualItems)
{
	int nLength = GetLineLength(nLineIndex);
	if (nLength <= 0)
		return dwCookie & COOKIE_EXT_COMMENT;

	LPCTSTR pszChars    = GetLineChars(nLineIndex);
	BOOL bFirstChar     = (dwCookie & ~COOKIE_EXT_COMMENT) == 0;
	BOOL bRedefineBlock = TRUE;
	BOOL bDecIndex  = FALSE;
	int nIdentBegin = -1;
	int I = 0;
	for (I = 0; ; I++)
	{
		if (bRedefineBlock)
		{
			int nPos = I;
			if (bDecIndex)
				nPos--;
			if (dwCookie & (COOKIE_COMMENT | COOKIE_EXT_COMMENT))
			{
				DEFINE_BLOCK(nPos, COLORINDEX_COMMENT);
			}
			else
			if (dwCookie & (COOKIE_CHAR | COOKIE_STRING))
			{
				DEFINE_BLOCK(nPos, COLORINDEX_STRING);
			}
			else
			if (dwCookie & COOKIE_PREPROCESSOR)
			{
				DEFINE_BLOCK(nPos, COLORINDEX_PREPROCESSOR);
			}
			else
			{
				DEFINE_BLOCK(nPos, COLORINDEX_NORMALTEXT);
			}
			bRedefineBlock = FALSE;
			bDecIndex      = FALSE;
		}

		if (I == nLength)
			break;

		if (dwCookie & COOKIE_COMMENT)
		{
			DEFINE_BLOCK(I, COLORINDEX_COMMENT);
			dwCookie |= COOKIE_COMMENT;
			break;
		}

		//	String constant "...."
		if (dwCookie & COOKIE_STRING)
		{
			if (pszChars[I] == '"' && (I == 0 || pszChars[I - 1] != '\\'))
			{
				dwCookie &= ~COOKIE_STRING;
				bRedefineBlock = TRUE;
			}
			continue;
		}

		//	Char constant '..'
		if (dwCookie & COOKIE_CHAR)
		{
			if (pszChars[I] == '\'' && (I == 0 || pszChars[I - 1] != '\\'))
			{
				dwCookie &= ~COOKIE_CHAR;
				bRedefineBlock = TRUE;
			}
			continue;
		}

		//	Extended comment /*....*/
		if (dwCookie & COOKIE_EXT_COMMENT)
		{
			if (I > 0 && pszChars[I] == '/' && pszChars[I - 1] == '*')
			{
				dwCookie &= ~COOKIE_EXT_COMMENT;
				bRedefineBlock = TRUE;
			}
			continue;
		}

		if (I > 0 && pszChars[I] == '/' && pszChars[I - 1] == '/')
		{
			DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
			dwCookie |= COOKIE_COMMENT;
			break;
		}

		//	Preprocessor directive #....
		if (dwCookie & COOKIE_PREPROCESSOR)
		{
			if (I > 0 && pszChars[I] == '*' && pszChars[I - 1] == '/')
			{
				DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
				dwCookie |= COOKIE_EXT_COMMENT;
			}
			continue;
		}

		//	Normal text
		if (pszChars[I] == '"')
		{
			DEFINE_BLOCK(I, COLORINDEX_STRING);
			dwCookie |= COOKIE_STRING;
			continue;
		}
		if (pszChars[I] == '\'')
		{
			DEFINE_BLOCK(I, COLORINDEX_STRING);
			dwCookie |= COOKIE_CHAR;
			continue;
		}
		if (I > 0 && pszChars[I] == '*' && pszChars[I - 1] == '/')
		{
			DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
			dwCookie |= COOKIE_EXT_COMMENT;
			continue;
		}

		if (bFirstChar)
		{
			if (pszChars[I] == '#')
			{
				DEFINE_BLOCK(I, COLORINDEX_PREPROCESSOR);
				dwCookie |= COOKIE_PREPROCESSOR;
				continue;
			}
			if (! isspace(pszChars[I]))
				bFirstChar = FALSE;
		}

		if (pBuf == NULL)
			continue;	//	We don't need to extract keywords,
						//	for faster parsing skip the rest of loop

		if (isalnum(pszChars[I]) || pszChars[I] == '_' || pszChars[I] == '.')
		{
			if (nIdentBegin == -1)
				nIdentBegin = I;
		}
		else
		{
			if (nIdentBegin >= 0)
			{
				if (IsCppKeyword(pszChars + nIdentBegin, I - nIdentBegin))
				{
					DEFINE_BLOCK(nIdentBegin, COLORINDEX_KEYWORD);
				}
				else
				if (IsCppNumber(pszChars + nIdentBegin, I - nIdentBegin))
				{
					DEFINE_BLOCK(nIdentBegin, COLORINDEX_NUMBER);
				}
				bRedefineBlock = TRUE;
				bDecIndex = TRUE;
				nIdentBegin = -1;
			}
		}
	}

	if (nIdentBegin >= 0)
	{
		if (IsCppKeyword(pszChars + nIdentBegin, I - nIdentBegin))
		{
			DEFINE_BLOCK(nIdentBegin, COLORINDEX_KEYWORD);
		}
		else
		if (IsCppNumber(pszChars + nIdentBegin, I - nIdentBegin))
		{
			DEFINE_BLOCK(nIdentBegin, COLORINDEX_NUMBER);
		}
	}

	if (pszChars[nLength - 1] != '\\')
		dwCookie &= COOKIE_EXT_COMMENT;
	return dwCookie;
}
