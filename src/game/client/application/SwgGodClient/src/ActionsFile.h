// ======================================================================
//
// ActionsFile.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ActionsFile_H
#define INCLUDED_ActionsFile_H

// ======================================================================

#include "Singleton/Singleton.h"

#include <qobject.h>

// ======================================================================

class ActionHack;

// ======================================================================
/**
 * ActionsFile is the resting place of various file-menu related actions
 */

class ActionsFile : public QObject, public Singleton<ActionsFile>
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	//lint -save
	//lint -e1925 // public data member
	ActionHack*  exit;

	//lint -restore

public slots:
	void onExit() const;

public:
	ActionsFile();
	~ActionsFile();

private:
	//disabled
	ActionsFile(const ActionsFile & rhs);
	ActionsFile& operator=(const ActionsFile & rhs);
};

// ======================================================================

#endif
