// ======================================================================
//
// Conversation.h
// asommers 2003-09-23
//
// copyright2003, sony online entertainment
// 
// ======================================================================

#ifndef INCLUDED_Conversation_H
#define INCLUDED_Conversation_H

// ======================================================================

class Iff;
class ScriptGroup;
class StringIdTracker;

// ======================================================================

class ConversationItem
{
public:
	explicit ConversationItem (StringIdTracker * stringIdTracker);
	virtual ~ConversationItem () = 0;

	virtual void reset ();
	virtual void load (Iff & iff);
	virtual void save (Iff & iff) const;

	bool getDebug () const;
	virtual void setDebug (bool debug);
	virtual bool hasChildren () const;
	virtual ConversationItem const * getParent () const;
	virtual ConversationItem * getParent ();
	void setParent (ConversationItem * parent);
	int getDepth() const;

	int getConditionFamilyId () const;
	void setConditionFamilyId (int conditionFamilyId);
	bool getNegateCondition () const;
	void setNegateCondition (bool negateCondition);
	bool getGroupEcho () const;
	void setGroupEcho (const bool groupEcho);
	int getActionFamilyId () const;
	void setActionFamilyId (int actionFamilyId);
	int getLabelFamilyId () const;
	void setLabelFamilyId (int labelFamilyId);
	int getLinkFamilyId () const;
	void setLinkFamilyId (int linkFamilyId);
	std::string const & getStringId() const;
	std::string const & getText () const;
	void setText (std::string const & text);
	std::string const & getNotes () const;
	void setNotes (std::string const & notes);
	bool getExpanded () const;
	void setExpanded (bool expanded);
	std::string const & getPlayerAnimation () const;
	void setPlayerAnimation (std::string const & playerAnimation);
	std::string const & getNpcAnimation () const;
	void setNpcAnimation (std::string const & npcAnimation);
	bool getUseProsePackage () const;
	void setUseProsePackage (bool useProsePackage);
	int getTokenTOFamilyId () const;
	void setTokenTOFamilyId (int tokenTOFamilyId);
	int getTokenDIFamilyId () const;
	void setTokenDIFamilyId (int tokenDIFamilyId);
	int getTokenDFFamilyId () const;
	void setTokenDFFamilyId (int tokenDFFamilyId);
	StringIdTracker * getStringIdTracker();
	void deleteStringIdTracker();

	//-- internal
	virtual ConversationItem const * getLink (int linkFamilyId) const;

private:

	ConversationItem ();

	void setStringId(std::string const & stringId);

	void load_0000 (Iff & iff);
	void load_0001 (Iff & iff);
	void load_0002 (Iff & iff);
	void load_0003 (Iff & iff);
	void load_0004 (Iff & iff);
	void load_0005 (Iff & iff);
	void load_0006 (Iff & iff);
	void load_0007 (Iff & iff);

	void save_0006 (Iff & iff) const;
	void save_0007 (Iff & iff) const;

private:

	int m_conditionFamilyId;
	bool m_negateCondition;
	int m_actionFamilyId;
	int m_labelFamilyId;
	int m_linkFamilyId;
	std::string m_stringId;
	std::string m_text;
	std::string m_notes;
	bool m_debug;
	bool m_expanded;
	bool m_groupEcho;
	std::string m_playerAnimation;
	std::string m_npcAnimation;
	bool m_useProsePackage;
	int m_tokenTOFamilyId;
	int m_tokenDIFamilyId;
	int m_tokenDFFamilyId;
	ConversationItem * m_parent;
	StringIdTracker * m_stringIdTracker;
};

// ----------------------------------------------------------------------

inline int ConversationItem::getDepth() const
{
	return getParent() ? 1 + getParent()->getDepth() : 0;
}

// ======================================================================

class ConversationResponse;
class ConversationBranch;

// ======================================================================

class ConversationBranch : public ConversationItem
{
public:

	explicit ConversationBranch (StringIdTracker * stringIdTracker);
	virtual ~ConversationBranch ();

	virtual void reset ();
	virtual void load (Iff & iff);
	virtual void save (Iff & iff) const;
	virtual void setDebug (bool debug);
	virtual bool hasChildren () const;

	int getNumberOfResponses () const;
	ConversationResponse const * getResponse (int index) const;
	ConversationResponse * getResponse (int index);
	void addResponse (ConversationResponse * response);
	void removeResponse (int index);
	void removeResponse (ConversationResponse const * response, bool doDelete);
	void promoteResponse (ConversationResponse const * response);
	void demoteResponse (ConversationResponse const * response);

	void setGroupEchoRecursive(const bool groupEcho);

	void setBranchId (int branchId) const;
	int getBranchId () const;

	//-- internal
	virtual ConversationItem const * getLink (int linkFamilyId) const;

private:

	ConversationBranch ();

	void load_0000 (Iff & iff);
	void load_0001 (Iff & iff);

private:

	typedef std::vector<ConversationResponse *> ResponseList;
	ResponseList m_responseList;

	int mutable m_branchId;
};

// ======================================================================

class ConversationResponse : public ConversationItem
{
public:

	explicit ConversationResponse (StringIdTracker * stringIdTracker);
	virtual ~ConversationResponse ();

	virtual void reset ();
	virtual void load (Iff & iff);
	virtual void save (Iff & iff) const;
	virtual void setDebug (bool debug);
	virtual bool hasChildren () const;

	int getNumberOfBranches () const;
	ConversationBranch const * getBranch (int index) const;
	ConversationBranch * getBranch (int index);
	void addBranch (ConversationBranch * branch);
	void removeBranch (int index);
	void removeBranch (ConversationBranch const * branch, bool doDelete);
	void promoteBranch (ConversationBranch const * branch);
	void demoteBranch (ConversationBranch const * branch);

	void setGroupEchoRecursive(const bool groupEcho);

	//-- internal
	virtual ConversationItem const * getLink (int linkFamilyId) const;

private:

	ConversationResponse ();

	void load_0000 (Iff & iff);
	void load_0001 (Iff & iff);

private:

	typedef std::vector<ConversationBranch *> BranchList;
	BranchList m_branchList;
};

// ======================================================================

class Conversation : public ConversationResponse
{
public:

	typedef std::set<std::string> LibrarySet;

public:

	Conversation ();
	virtual ~Conversation ();

	virtual void reset ();

	bool load (char const * fileName);
	bool save (char const * fileName) const;

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
	std::string const & getTriggerText () const;
	void setTriggerText (std::string const & triggerText);
	LibrarySet & getLibrarySet ();
	LibrarySet const & getLibrarySet () const;

private:

	Conversation (Conversation &);
	Conversation & operator= (Conversation &);

	void load_old (Iff & iff);
	void load_0000 (Iff & iff);
	void load_0001 (Iff & iff);
	void load_0002 (Iff & iff);

private:

	ScriptGroup * const m_conditionGroup;
	ScriptGroup * const m_actionGroup;
	ScriptGroup * const m_labelGroup;
	ScriptGroup * const m_tokenTOGroup;
	ScriptGroup * const m_tokenDIGroup;
	ScriptGroup * const m_tokenDFGroup;
	std::string m_triggerText;
	LibrarySet m_librarySet;
};

// ======================================================================

#endif
