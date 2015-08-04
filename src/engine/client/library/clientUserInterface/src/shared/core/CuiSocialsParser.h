// ======================================================================
//
// CuiSocialsParser.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiSocialsParser_H
#define INCLUDED_CuiSocialsParser_H

// ======================================================================

class Object;
class LocalizedStringTable;

#include "sharedCommandParser/CommandParser.h"

/**
* The CuiSocialsParser interprets an input string, determining mood, spatial chat type,
* and target and sends a formatted chat messages to the CuiSocialsManager if applicable.
* 
*/

class CuiSocialsParser : public CommandParser
{
public:

	enum ParseResult
	{
		PR_NONE,
		PR_ERROR,
		PR_NO_SUCH_SOCIAL,
		PR_NO_SUCH_TARGET,
		PR_TARGET_NEEDED,
		PR_OK
	};

	static void           install ();
	static void           remove  ();

	                      CuiSocialsParser    ();
	virtual              ~CuiSocialsParser    ();
	virtual bool          performParsing      (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node);

	static bool           preparseEmoticons   (const Unicode::String & str, Unicode::String & result);

	static void           performSocial       (const Unicode::String & targetName, uint32 social_id, Unicode::String & result);
	static void           performSocialClient (const Object & actor, const Unicode::String & targetName, const std::string & socialName, Unicode::String & result);
	static NetworkId      findTarget          (const Object & actor, const Unicode::String & targetName, Unicode::String & result);
private:
	                      CuiSocialsParser (const CuiSocialsParser & rhs);
	CuiSocialsParser &    operator=    (const CuiSocialsParser & rhs);

};

// ======================================================================

#endif
