// ======================================================================
//
// SwgConversationEditor.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgConversationEditor_H
#define INCLUDED_SwgConversationEditor_H

// ======================================================================

#include "Resource.h"

class SwgConversationEditorDoc;

// ======================================================================

class SwgConversationEditorApp : public CWinApp
{
public:

	static CString const getVersion();

public:

	SwgConversationEditorApp();
	virtual ~SwgConversationEditorApp();

	//{{AFX_VIRTUAL(SwgConversationEditorApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(SwgConversationEditorApp)
	afx_msg void OnAppAbout();
	afx_msg void OnToolsReloaddictionary();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	void onOpenDefaultViews (SwgConversationEditorDoc * const document);

	void onViewConversation (SwgConversationEditorDoc * const document);
	void onViewScript (SwgConversationEditorDoc * const document);
	void onViewWarning (SwgConversationEditorDoc * const document);
	void onViewShell (SwgConversationEditorDoc * const document);

	bool isValidWord (CString const & word) const;
	
private:

	bool loadDictionary ();
	bool loadDictionary (char const * fileName);

private:

	CString m_currentDirectory;

	CMultiDocTemplate * m_conversationTemplate;
	CMultiDocTemplate * m_scriptGroupTemplate;
	CMultiDocTemplate * m_warningTemplate;
	CMultiDocTemplate * m_shellTemplate;

	typedef std::set<CString> Dictionary;
	Dictionary m_dictionary;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
