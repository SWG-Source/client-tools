// ======================================================================
//
// CuiActionManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiActionManager_H
#define INCLUDED_CuiActionManager_H

// ======================================================================

class CuiAction;
class MessageQueue;

//-----------------------------------------------------------------

class CuiActionManager
{
public:

	struct ActionInfo
	{
		CuiAction * m_action;
		bool        m_own;

		ActionInfo (CuiAction * action, bool own) : m_action (NON_NULL (action)), m_own (own) {}
	private:
		ActionInfo ();
	};

	typedef stdmap<std::string, ActionInfo>::fwd ActionMap;
	typedef stdmap<int /*index*/, ActionMap*>::fwd ActionMapMap;

	static ActionMap * getActionMap();
	static ActionMap * getActionMap(int index);
	static void install ();
	static void remove ();

	static bool              performAction (const std::string & id, const Unicode::String & params);
	static const CuiAction * findAction    (const std::string & id);
	static const CuiAction * findAction    (const std::string & id, int index);
	static CuiAction *       removeAction  (const std::string & id);
	static int               removeAction  (const CuiAction * action);
	static bool              addAction     (const std::string & id, CuiAction * action, bool own, bool replace = false);
	static void setCurrentActionMap(int index);
	static void resetActionMapIndex();
private:
	CuiActionManager (const CuiActionManager & rhs);
	CuiActionManager & operator= (const CuiActionManager & rhs);
	static ActionMap * createActionMap(int index);

private:
	static ActionMapMap *                         ms_actionMapMap;
};

// ======================================================================

#endif
