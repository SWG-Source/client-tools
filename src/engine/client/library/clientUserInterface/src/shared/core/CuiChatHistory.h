//======================================================================
//
// CuiChatHistory.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiChatHistory_H
#define INCLUDED_CuiChatHistory_H

#include "UITypes.h"

//======================================================================

class CuiChatHistory
{
public:

	static void install ();
	static void remove  ();
	static void clear   ();
	static void load    ();
	static void save    ();

	static void update       (float deltaTimeSecs);
	static void setDirty     (bool dirty);
	static bool isDirty();

	static void saveSize     (const std::string & owner, const UISize & size);
	static void saveLocation (const std::string & owner, const UIPoint & location);
	static void saveString   (const std::string & owner, const std::string & key, const Unicode::String & value);
	static void saveData     (const std::string & owner, const std::string & key, const std::string & value);
	static void saveInteger  (const std::string & owner, const std::string & key, const int value);
	static void saveBoolean  (const std::string & owner, const std::string & key, const bool value);

	static bool loadSize     (const std::string & owner, UISize & size);
	static bool loadLocation (const std::string & owner, UIPoint & location);
	static bool loadString   (const std::string & owner, const std::string & key, Unicode::String & value);
	static bool loadData     (const std::string & owner, const std::string & key, std::string & value);
	static bool loadInteger  (const std::string & owner, const std::string & key, int & value);
	static bool loadBoolean  (const std::string & owner, const std::string & key, bool & value);

	static void clearDataForOwner(std::string const & owner);

	static void resetForPlayer ();
	static bool isResettingForPlayer ();

	static void setPrefixString(std::string const & prefixString);
	static bool isNewPlayer();

	static void enableAutomaticSave(bool enable);
};

//======================================================================

#endif
