// ======================================================================
//
// SwgCuiCommandParserDefault.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommandParserDefault_H
#define INCLUDED_SwgCuiCommandParserDefault_H

#include "sharedCommandParser/CommandParser.h"

class CommandParserHistory;
class CuiAliasHandler;

// ======================================================================

class SwgCuiCommandParserDefault : public CommandParser
{
public:
	typedef stdmap<String_t, String_t>::fwd AliasMap_t;
	typedef stdset<String_t>::fwd           AliasGuardSet_t;


	explicit                     SwgCuiCommandParserDefault (CommandParserHistory * history);
	                            ~SwgCuiCommandParserDefault ();
	virtual bool                 performParsing             (const NetworkId & userId, const StringVector_t & argv, const Unicode::String & originalCommand, Unicode::String & result, const CommandParser * node);

	void                         setAliasHandler            (CuiAliasHandler * aliasHandler);

	static bool setAliasStatic(const std::string & key, const std::string & value);
	static bool setAliasStatic(const String_t & key, const String_t & value);
	static bool removeAliasStatic(const Unicode::String & key);

private:

	                             SwgCuiCommandParserDefault ();
	                             SwgCuiCommandParserDefault (const SwgCuiCommandParserDefault & rhs);
	SwgCuiCommandParserDefault & operator=                  (const SwgCuiCommandParserDefault & rhs);

	bool                         removeAliasInternal(const Unicode::String & key);

	bool                         setAliasInternal(const Unicode::String & key, const Unicode::String & value);
	bool                         setAliasInternal(const std::string & key, const std::string & value);

private:

	CommandParserHistory * m_history;

	int8                   m_aliasGuard;

	AliasGuardSet_t *      m_aliasGuardSet;

	std::string            m_aliasFilename;

	CuiAliasHandler *      m_aliasHandler;
};

// ======================================================================

#endif
