// ======================================================================
//
// UIBuilderHistory.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UIBuilderHistory_H
#define INCLUDED_UIBuilderHistory_H

// ======================================================================

class UIBuilderHistory
{
public:
	
	static void install ();
	static void remove ();

	static bool back (std::string & path );
	static bool forward (std::string & path);
	static void pushNode (const std::string & path);

	static bool backValid ();
	static bool forwardValid ();

	static bool isInstalled();

private:
	UIBuilderHistory (const UIBuilderHistory & rhs);
	UIBuilderHistory & operator= (const UIBuilderHistory & rhs);
};

// ======================================================================

#endif
