//======================================================================
//
// CuiStringGrammarManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringGrammarManager_H
#define INCLUDED_CuiStringGrammarManager_H

//======================================================================

class StringId;
class ClientObject;
class LocalizedString;

//----------------------------------------------------------------------

class CuiStringGrammarManager
{
public:
	static bool getRawMessage       (const StringId & id, const ClientObject * source, const ClientObject * target, Unicode::String & str);
	static bool getRawMessage       (const LocalizedString & locstr, const ClientObject * source, const ClientObject * target, Unicode::String & result);
};

//======================================================================

#endif
