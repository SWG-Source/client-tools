// ======================================================================
//
// ScriptView.h
// asommers 2003-09-23
//
// copyright2003, sony online entertainment
// 
// ======================================================================

#ifndef INCLUDED_ScriptView_H
#define INCLUDED_ScriptView_H

// ======================================================================

#include "CCrystalEditView.h"
#include "CCrystalTextBuffer.h"

class ScriptGroup;

// ======================================================================

class ScriptView : public CCrystalEditView
{
protected:

	ScriptView();           
	DECLARE_DYNCREATE(ScriptView)

	//{{AFX_VIRTUAL(ScriptView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	virtual ~ScriptView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(ScriptView)
	afx_msg void OnChange();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void editCondition (int conditionFamilyId);
	void editAction (int actionFamilyId);
	void editTokenTO (int tokenTOFamilyId);
	void editTokenDF (int tokenDFFamilyId);
	void editTokenDI (int tokenDIFamilyId);
	void editTrigger ();
	void editNothing ();

protected:

	CCrystalTextBuffer * LocateTextBuffer ();

private:

	enum Type
	{
		T_nothing,
		T_condition,
		T_action,
		T_tokenTO,
		T_tokenDF,
		T_tokenDI,
		T_trigger
	};

	class TextBuffer : public CCrystalTextBuffer
    {
    public:

        TextBuffer (CCrystalTextView * const view) :
			CCrystalTextBuffer (),
			m_view (view)
		{
		}

        virtual void SetModified (BOOL bModified)
		{
			m_view->GetDocument ()->SetModifiedFlag (bModified);
		}

		void SetText (CString text)
		{
			FreeAll ();
			InitNew ();

			int endLine;
			int endChar;
			InsertText(m_view, 0, 0, text, endLine, endChar);

			ClearUndo ();
		}

		CString GetText ()
		{
			CString result;
			for (int i = 0; i < GetLineCount (); ++i)
			{
				CString line = GetLineChars (i);
				line = line.Left (GetLineLength (i));
				result += line + "\r\n";
			}

			//-- strip any unnecessary line ends
			while (result.GetLength () >= 3 && result [result.GetLength () - 3] == '\n' && result [result.GetLength () - 2] == '\r' && result [result.GetLength () - 1] == '\n')
				result = result.Left (result.GetLength () - 2);

			return result;
		}

	private:

		CCrystalTextView * const m_view;
    };

private:

	ScriptGroup * getScriptGroup ();
	void fixFunctionName ();

	DWORD ParseLine(DWORD dwCookie, int nLineIndex, TEXTBLOCK *pBuf, int &nActualItems);

private:

	CFont * m_font;
	int m_familyId;
	Type m_type;

	TextBuffer m_textBuffer;
	LOGFONT m_lf;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
