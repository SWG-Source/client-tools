// ======================================================================
//
// SwgConversationEditorDoc.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgConversationEditorDoc_H
#define INCLUDED_SwgConversationEditorDoc_H

// ======================================================================

#include "Conversation.h"

class ConversationFrame;
class ScriptFrame;
class ScriptGroup;
class ScriptShellFrame;
class StringIdTracker;
class WarningFrame;

// ======================================================================

class SwgConversationEditorDoc : public CDocument
{
protected: 

	SwgConversationEditorDoc();
	DECLARE_DYNCREATE(SwgConversationEditorDoc)

public:

	//{{AFX_VIRTUAL(SwgConversationEditorDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

public:

	virtual ~SwgConversationEditorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//{{AFX_MSG(SwgConversationEditorDoc)
	afx_msg void OnFileImport();
	afx_msg void OnUpdateButtonCompile(CCmdUI* pCmdUI);
	afx_msg void OnButtonCompiledebug();
	afx_msg void OnButtonCompilerelease();
	afx_msg void OnButtonP4edit();
	afx_msg void OnButtonScan();
	afx_msg void OnButtonSpellcheck();
	afx_msg void OnButtonFind();
	afx_msg void OnFileExport();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	typedef std::vector<ConversationItem const *> ConversationItemList;

	enum Hints
	{
		H_savingDocument,
		H_textChanged,
		H_fileImported,
		H_scriptChanged,
		H_conversationItemChanged,
		H_debugChanged,
		H_conditionChanged,
		H_negateConditionChanged,
		H_groupEchoChanged,
		H_actionChanged,
		H_labelChanged,
		H_linkChanged,
		H_shellCompile,
		H_shellP4edit,
		H_triggerChanged
	};

	struct ItemData
	{
	public:

		enum Type
		{
			T_unknown,
			T_branch,
			T_response
		};

	public:

		Type m_type;
		ConversationItem * m_conversationItem;
		HTREEITEM m_treeItem;

	public:

		ItemData ();
		~ItemData ();

		CString const getText () const;
	};

	class ShellData : public CObject
	{
	public:

		ShellData () :
			CObject (),
			m_debug (false),
			m_shortFileName (),
			m_fullScriptFileName (),
			m_fullStringTableFileName ()
		{
		}

	public:

		bool m_debug;

		CString m_shortFileName;
		CString m_fullScriptFileName;
		CString m_fullStringTableFileName;
	};

public:

	static CString const & getScriptPreNameDeclarationConditionString ();
	static CString const & getScriptPostNameDeclarationConditionString ();
	static CString const & getScriptBodyConditionString ();
	static CString const & getScriptPreNameDeclarationActionString ();
	static CString const & getScriptPostNameDeclarationActionString ();
	static CString const & getScriptBodyActionString ();
	static CString const & getScriptPreNameDeclarationTokenTOString ();
	static CString const & getScriptPostNameDeclarationTokenTOString ();
	static CString const & getScriptBodyTokenTOString ();
	static CString const & getScriptPreNameDeclarationTokenDIString ();
	static CString const & getScriptPostNameDeclarationTokenDIString ();
	static CString const & getScriptBodyTokenDIString ();
	static CString const & getScriptPreNameDeclarationTokenDFString ();
	static CString const & getScriptPostNameDeclarationTokenDFString ();
	static CString const & getScriptBodyTokenDFString ();

	typedef CString const (*CreateEmptyScriptFunction) (CString const & scriptName);
	static CString const createEmptyConditionScript (CString const & scriptName);
	static CString const createEmptyActionScript (CString const & scriptName);
	static CString const createEmptyTokenTOScript (CString const & scriptName);
	static CString const createEmptyTokenDIScript (CString const & scriptName);
	static CString const createEmptyTokenDFScript (CString const & scriptName);

	static CString const getDefaultTriggerText ();

	static CString const fixConditionFunctionName (CString const & text, CString const & prefix, ScriptGroup const * scriptGroup, int familyId);
	static CString const fixActionFunctionName (CString const & text, CString const & prefix, ScriptGroup const * scriptGroup, int familyId);
	static CString const fixTokenTOFunctionName (CString const & text, CString const & prefix, ScriptGroup const * scriptGroup, int familyId);
	static CString const fixTokenDIFunctionName (CString const & text, CString const & prefix, ScriptGroup const * scriptGroup, int familyId);
	static CString const fixTokenDFFunctionName (CString const & text, CString const & prefix, ScriptGroup const * scriptGroup, int familyId);

public:

	Conversation * getConversation ();
	Conversation const * getConversation () const;
	ScriptGroup * getConditionGroup ();
	ScriptGroup const * getConditionGroup () const;
	ScriptGroup * getActionGroup ();
	ScriptGroup const * getActionGroup () const;
	ScriptGroup * getLabelGroup ();
	ScriptGroup const * getLabelGroup () const;
	ScriptGroup * getTokenTOGroup ();
	ScriptGroup const * getTokenTOGroup () const;
	ScriptGroup * getTokenDIGroup ();
	ScriptGroup const * getTokenDIGroup () const;
	ScriptGroup * getTokenDFGroup ();
	ScriptGroup const * getTokenDFGroup () const;
	Conversation::LibrarySet & getLibrarySet ();
	Conversation::LibrarySet const & getLibrarySet () const;
	StringIdTracker * getStringIdTracker ();

	ConversationFrame * getConversationFrame ();
	void setConversationFrame (ConversationFrame * conversationFrame);
	ScriptFrame * getScriptFrame ();
	void setScriptFrame (ScriptFrame * scriptFrame);
	WarningFrame * getWarningFrame ();
	void setWarningFrame (WarningFrame * warningFrame);
	ScriptShellFrame * getScriptShellFrame ();
	void setScriptShellFrame (ScriptShellFrame * scriptShellFrame);

	void buildConditionGroupComboBox (CComboBox & comboBox, int selectedFamilyId) const;
	void buildActionGroupComboBox (CComboBox & comboBox, int selectedFamilyId) const;
	void buildLabelGroupComboBox (CComboBox & comboBox, int selectedFamilyId, bool skipRoot) const;
	void buildTokenTOGroupComboBox (CComboBox & comboBox, int selectedFamilyId) const;
	void buildTokenDIGroupComboBox (CComboBox & comboBox, int selectedFamilyId) const;
	void buildTokenDFGroupComboBox (CComboBox & comboBox, int selectedFamilyId) const;

	void addError (CString const & error, ConversationItem const * conversationItem);
	void addWarning (CString const & warning, ConversationItem const * conversationItem);
	void addComment (CString const & comment, ConversationItem const * conversationItem);

	bool findConditions (int conditionFamilyId, ConversationItemList & conversationItemList) const;
	bool findActions (int actionFamilyId, ConversationItemList & conversationItemList) const;
	bool findLabels (int labelFamilyId, ConversationItemList & conversationItemList) const;
	bool findLinks (int labelFamilyId, ConversationItemList & conversationItemList) const;
	bool findLabelsOrLinks (int labelFamilyId, ConversationItemList & conversationItemList) const;

	bool conditionInUse (int conditionFamilyId) const;
	bool actionInUse (int actionFamilyId) const;
	bool tokenTOInUse (int tokenTOFamilyId) const;
	bool tokenDIInUse (int tokenDIFamilyId) const;
	bool tokenDFInUse (int tokenDFFamilyId) const;
	bool labelInUseAsLabel (int labelFamilyId) const;
	bool labelInUseAsLink (int labelFamilyId) const;
	bool labelInUseAsLabelOrLink (int labelFamilyId) const;

	std::pair <bool, bool> compile (bool spellCheck, bool scan, bool compile, bool debug, bool test = false);

private:

	void buildScriptGroupComboBox (CComboBox & comboBox, ScriptGroup const * scriptGroup, int selectedFamilyId, bool skipRoot) const;

	void prepareCompileBranch (ConversationBranch const * branch, ConversationItemList & conversationItemList, int & branchId);
	void prepareCompileResponse (ConversationResponse const * response, ConversationItemList & conversationItemList, int & branchId);

	bool writeScript (CString const & scriptName, CString const & fileName, ConversationItemList const & conversationItemList, bool const debug) const;
	void writeLibraries (CStdioFile & outfile) const;
	void writeResponseChildBranches (CStdioFile & outfile, CString const & fileName, int indentLevel, ConversationResponse const * response, bool start, bool const debug) const;
	void writeProsePackage (CStdioFile & outfile, CString const & fileName, int indentLevel, ConversationItem const * const conversationItem) const;
	bool writeStringTable (CString const & stringTableName, CString const & fileName, ConversationItemList const & conversationItemList) const;

	void findConditionBranch (ConversationBranch const * branch, int conditionFamilyId, ConversationItemList & conversationItemList) const;
	void findConditionResponse (ConversationResponse const * response, int conditionFamilyId, ConversationItemList & conversationItemList) const;
	void findActionBranch (ConversationBranch const * branch, int actionFamilyId, ConversationItemList & conversationItemList) const;
	void findActionResponse (ConversationResponse const * response, int actionFamilyId, ConversationItemList & conversationItemList) const;
	void findTokenTOBranch (ConversationBranch const * branch, int tokenTOFamilyId, ConversationItemList & conversationItemList) const;
	void findTokenTOResponse (ConversationResponse const * response, int tokenTOFamilyId, ConversationItemList & conversationItemList) const;
	void findTokenDIBranch (ConversationBranch const * branch, int tokenDIFamilyId, ConversationItemList & conversationItemList) const;
	void findTokenDIResponse (ConversationResponse const * response, int tokenDIFamilyId, ConversationItemList & conversationItemList) const;
	void findTokenDFBranch (ConversationBranch const * branch, int tokenDFFamilyId, ConversationItemList & conversationItemList) const;
	void findTokenDFResponse (ConversationResponse const * response, int tokenDFFamilyId, ConversationItemList & conversationItemList) const;
	void findLabelBranch (ConversationBranch const * branch, int labelFamilyId, ConversationItemList & conversationItemList) const;
	void findLabelResponse (ConversationResponse const * response, int labelFamilyId, ConversationItemList & conversationItemList) const;
	void findLinkBranch (ConversationBranch const * branch, int linkFamilyId, ConversationItemList & conversationItemList) const;
	void findLinkResponse (ConversationResponse const * response, int linkFamilyId, ConversationItemList & conversationItemList) const;

	ConversationItem const * findLabel (int labelFamilyId) const;
	ConversationItem const * findLabelBranch (ConversationBranch const * branch, int labelFamilyId) const;
	ConversationItem const * findLabelResponse (ConversationResponse const * response, int labelFamilyId) const;

	typedef std::vector<CString> StringList;
	void spellCheck (CString const & text, StringList & stringList, ConversationItem const * conversationItem);
	void spellCheckBranch (ConversationBranch const * branch);
	void spellCheckResponse (ConversationResponse const * response);
	void linkCheckBranch (ConversationBranch const * branch);
	void linkCheckResponse (ConversationResponse const * response);
	void labelCheckBranch (ConversationBranch const * branch);
	void labelCheckResponse (ConversationResponse const * response);
	void findTextBranch (ConversationBranch const * branch, CString const & findText, bool matchCase, bool wholeWordOnly);
	void findTextResponse (ConversationResponse const * response, CString const & findText, bool matchCase, bool wholeWordOnly);

	void verifyLabels ();
	void verifyLinks ();

	void fixupTriggerText (CString const & fileName);

private:

	Conversation * const m_conversation;

	ConversationFrame * m_conversationFrame;
	ScriptFrame * m_scriptFrame;
	WarningFrame * m_warningFrame;
	ScriptShellFrame * m_scriptShellFrame;

	int m_numberOfWarnings;
	int m_numberOfErrors;
	int m_numberOfComments;
};

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 

