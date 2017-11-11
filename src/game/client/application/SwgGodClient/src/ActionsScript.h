// ======================================================================
//
// ActionsScript.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ActionsScript_H
#define INCLUDED_ActionsScript_H

// ======================================================================

#include "Singleton/Singleton.h"

// ======================================================================

class ActionHack;

//----------------------------------------------------------------------

/**
* ActionsScript is the resting place of various script-menu related actions
*/
class ActionsScript : public QObject, public Singleton<ActionsScript>
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762

public:

	struct DragMessages
	{
		static const char* const SCRIPT_DRAGGED;
	};

	//lint -save
	//lint -e1925 // public data member
	ActionHack*             create;
	ActionHack*             view;
	ActionHack*             edit;
	ActionHack*             revert;
	ActionHack*             refresh;
	ActionHack*             submit;
	ActionHack*             compile;
	ActionHack*             serverReload;
	ActionHack*             removeScript;
	ActionHack*             removeObjvar;
	ActionHack*             setObjvar;

	//lint -restore

public slots:
	void onScriptPathSelectionChanged(const std::string& path, bool isFile, bool isNew, bool isEdit);
	bool getSelectedScript           (std::string& path) const;

private slots:
	void doEditFile    (const std::string& filename) const;
	void onRevert      ();
	void onView        () const;
	void onEdit        ();
	void onNew         ();
	void onSubmit      ();
	void onCompile     () const;
	void onServerReload() const;

public:
	ActionsScript ();
	~ActionsScript();

private:
	//disabled
	ActionsScript(const ActionsScript& rhs);
	ActionsScript& operator=(const ActionsScript& rhs);

private:
	static const std::string convertToClasspath(const std::string& path);

	const std::string getScriptFilePath() const;
	const std::string getClassFilePath() const;

	std::string   m_selectedPath;
	bool          m_isFile;
	bool          m_isNew;
	bool          m_isEdit;

};

// ======================================================================

#endif

